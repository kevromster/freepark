JsApp = function()
{
    var FREEPARK_SERVER = 'http://18.196.226.205:8000';
    var FIND_PARKINGS_NEAR_BOUNDARY_METERS = 200;
    var UPDATE_FREE_PARKING_PLACES_INTERVAL_SECS = 60;

    var BALLOON_PARKING_PLACE_IMAGE_MAX_SIZE = 250; //px
    var BALLOON_PARKING_PLACE_TEXT_HEIGHT = 80; //px

    var mapInstance;
    var logElement;

    var shownParkPlacesCluster;

    var userLocation;  // current user location coordinates
    var userLocationPlacemark;

    var activeRoute;
    var isTrackingRoute = false; // true if we track user's moving along the active route

    function getParkingComplexityName(complexity) {
        if (complexity == 1)
            return 'места достаточно';
        if (complexity == 2)
            return 'может быть узковато';
        if (complexity == 3)
            return 'осторожно, узко!';
        return 'нет данных';
    }

    function getParkingComplexityIconName(complexity) {
        if (complexity == 2)
            return 'parking_place_accurate.png';
        if (complexity == 3)
            return 'parking_place_experienced.png';
        return 'parking_place_newbie.png';
    }

    function timeNow() {
        var cur = new Date();
        var res =
            ((cur.getHours() < 10) ? '0' : '') + cur.getHours() + ':' +
            ((cur.getMinutes() < 10) ? '0' : '') + cur.getMinutes() + ':' +
            ((cur.getSeconds() < 10) ? '0' : '') + cur.getSeconds();
        return res;
    }

    function log(message) {
        console.log(message);
        logElement.innerHTML = message + '<br/>' + logElement.innerHTML;
    }

    function httpGetAsync(theUrl, successCallback, errorCallback) {
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.onreadystatechange = function() {
            if (xmlHttp.readyState == 4) {
                if (xmlHttp.status >= 200 && xmlHttp.status < 300)
                    successCallback(xmlHttp.responseText);
                else if (errorCallback !== undefined)
                    errorCallback(xmlHttp.responseText);
            }
        }
        xmlHttp.open("GET", theUrl, true); // true for asynchronous
        xmlHttp.send(null);
    }

    function showMessageBalloon(latitude, longitude, messageBody, messageTitle) {
        log(messageBody);
        var options = {contentBody : messageBody};
        if (messageTitle) {
            log(messageTitle);
            options.contentHeader = messageTitle;
        }
        mapInstance.balloon.open([latitude, longitude], options);
    }

    function onFreeParkingPlacesFound(nearPtLat, nearPtLon, parkingPlaces) {
        // nearPtLat/nearPtLon - point near which free parking places were searched for.
        // Always at least one parking place must be in the list, otherwise it is wrong method usage!

        var curTime = timeNow();
        log('Free parking places found: quantity = ' + parkingPlaces.length + ' for the time: ' + curTime);

        if (mapInstance.balloon.isOpen()) {
            mapInstance.balloon.close();
        }

        if (!shownParkPlacesCluster) {
            shownParkPlacesCluster = new ymaps.Clusterer({
                clusterHideIconOnBalloonOpen: false,
                geoObjectHideIconOnBalloonOpen: false,
                clusterBalloonContentLayout: 'cluster#balloonCarousel',
                clusterBalloonPagerType: 'marker',
                clusterBalloonContentLayoutWidth: BALLOON_PARKING_PLACE_IMAGE_MAX_SIZE,
                clusterBalloonContentLayoutHeight: BALLOON_PARKING_PLACE_IMAGE_MAX_SIZE,
                clusterIconContentLayout: ymaps.templateLayoutFactory.createClass(
                    '<div style="color:#FFFFFF; font-weight:bold; position:relative; left:5px; top:13px;">{{properties.geoObjects.length}}</div>'
                ),
                clusterIcons: [{
                    href: 'parking_place_newbie.png',  // always treat many parkings as for newbies
                    size: [48, 48],
                    offset: [-24, -24]
                }],
                clusterNumbers: []
            });
            mapInstance.geoObjects.add(shownParkPlacesCluster);
        }

        // remove old placemarks
        shownParkPlacesCluster.removeAll();

        for (var idx in parkingPlaces) {
            var parkingPlace = parkingPlaces[idx];
            shownParkPlacesCluster.add(
                new ymaps.Placemark(
                    [parkingPlace.latitude, parkingPlace.longitude],
                    {
                        balloonContentBody: [
                            '<div id="parking-place-info-div" style="text-align:center;vertical-align:middle;">',
                            '<img id="parking-place-image" src="' + parkingPlace.image +
                            '" alt="no-image" style="max-width:' +
                            BALLOON_PARKING_PLACE_IMAGE_MAX_SIZE + 'px;max-height:' +
                            BALLOON_PARKING_PLACE_IMAGE_MAX_SIZE + 'px;"' +
                            ' onload="JsApp.onParkingPlaceImageLoaded();">',
                            '<br/>',
                            'Актуально на: ' + curTime,
                            '<br/>',
                            'Сложность: ' + getParkingComplexityName(parkingPlace.parking_complexity),
                            '<br/>',
                            '<a href="javascript:void(0)" onclick="JsApp.buildRoute(' +
                            [parkingPlace.latitude, parkingPlace.longitude].join(',') +
                            ');">Проложить маршрут</a>',
                            '</div>'
                        ].join(''),
                        iconContent: idx.toString()
                    },
                    {
                        iconLayout: 'default#image',
                        iconImageHref: getParkingComplexityIconName(parkingPlace.parking_complexity),
                        iconImageSize: [24, 24],
                        iconImageOffset: [-12, -12]
                    }
                )
            );
        }

        if (isTrackingRoute) {
            var closestPlace = parkingPlaces[0];
            var epsilon = 0.00001;
            if (Math.abs(nearPtLat - closestPlace.latitude) > epsilon ||
                Math.abs(nearPtLon - closestPlace.longitude) > epsilon) {
                buildRoute(closestPlace.latitude, closestPlace.longitude, true);
            }
        }
    }

    function showCurrentTraffic() {
        var trafficControl = mapInstance.controls.get('trafficControl');
        trafficControl.getProvider('traffic#actual').state.set('infoLayerShown', true);
        trafficControl.showTraffic();
    }

    function onUserLocationChanged(newLocation) {
        userLocation = newLocation;

        if (!userLocationPlacemark) {
            userLocationPlacemark = new ymaps.Placemark(
                userLocation,
                {},
                { preset: 'islands#geolocationIcon' }
            );
            mapInstance.geoObjects.add(userLocationPlacemark);
        } else {
            userLocationPlacemark.geometry.setCoordinates(userLocation);
        }
    }

    function determineUserLocation(onSuccess, onError) {
        log('determining current user location...');

        var errorTimeout = 10 * 1000;
        var currentTimeout = 0;

        var tryTakeLocation = function(onSuccess, onError) {
            var rawLocation = JavaJsCommunicator.getCurrentLocation();
            if (rawLocation != "") {
                log('determined location: ' + rawLocation);
                onUserLocationChanged(JSON.parse(rawLocation));
                onSuccess();
            } else if (currentTimeout > errorTimeout) {
                log('Cannot determine user location');
                if (onError) {
                    onError({message:'Too long to respond'});
                }
            } else {
                currentTimeout += 2000;
                // try to take location again after a timeout
                setTimeout(tryTakeLocation, currentTimeout, onSuccess, onError);
            }
        };

        tryTakeLocation(onSuccess, onError);
    }

    function zoomToUserLocation() {
        mapInstance.panTo(userLocation).then(function() {
            mapInstance.setZoom(16).then(function() {
                // pan again for better accuracy
                mapInstance.panTo(userLocation);
            });
        });
    }

    function zoomToUserLocationAtStart() {
        determineUserLocation(function() {
            zoomToUserLocation();
        });
    }

    function buildRouteFromTo(fromPosition, lat, lon, isAutoRebuild) {
        log('building route from ' + JSON.stringify(fromPosition) + ' to [' + lat + ',' + lon + ']...');

        ymaps.route(
            [fromPosition, [lat, lon]],
            {
                avoidTrafficJams: true,
                mapStateAutoApply: !isTrackingRoute // do not auto-pan map to the route if tracking another one
            }
        ).then(
            function(route) {
                if (mapInstance.balloon.isOpen()) {
                    mapInstance.balloon.close();
                }

                if (activeRoute) {
                    // remove old route before adding new one
                    mapInstance.geoObjects.remove(activeRoute);
                }

                activeRoute = route;
                mapInstance.geoObjects.add(activeRoute);
                log('Route added to the map.');

                if (!isTrackingRoute || !isAutoRebuild) {
                    // not tracking yet, suggest user to track
                    mapInstance.balloon.open(
                        [lat, lon],
                        '<a href="javascript:void(0)" onclick="JsApp.trackRoute(' +
                        [lat, lon].join(',') + ');">Поехали!</a>',
                        { panelMaxMapArea: Infinity } // always open in panel mode at the bottom of the map
                    ).then(function(res) {
                        mapInstance.balloon.events.once('close', function(e) {
                            // reset panel model of the balloon
                            mapInstance.balloon.setOptions({ panelMaxMapArea: undefined });
                        });
                    });
                }
            },
            function(error) {
                if (!isTrackingRoute || !isAutoRebuild) {
                    showMessageBalloon(lat, lon, 'Ошибка построения маршрута: ' + error.message);
                } else {
                    log('Ошибка построения маршрута: ' + error.message);
                }
            }
        );
    }

    function createMap(mapElementId) {
        mapInstance = new ymaps.Map(
            mapElementId,
            {
                center: [55.02, 82.92], // Novosibirsk
                zoom: 10
            },
            {
                searchControlProvider: 'yandex#search',
                autoFitToViewport: 'always'
            }
        );

        // open balloon on click with suggestion to find free parkings
        mapInstance.events.add('click', function(e) {
            if (mapInstance.balloon.isOpen()) {
                // balloon already opened, close it
                mapInstance.balloon.close();
                return;
            }

            var coords = e.get('coords');

            mapInstance.balloon.open(coords, {
                contentBody:
                    '<a href="javascript:void(0)" onclick="JsApp.findFreeParkings(' +
                    coords.join(',') + ');">Найти парковки рядом</a>'
            });
        });

        // update our stored user location coordinates when user clicks on on geolocation control
        mapInstance.controls.get('geolocationControl').events.add(
            'locationchange',
            function (event) {
                log('location changed, new position: ' + JSON.stringify(event.get('position')));
                onUserLocationChanged(event.get('position'));
        });

        log('Map created');
    }

    function runPeriodicFreeParkingPlaceCheck(lat, lon) {
        // lat/lon here is a coordinate of the target free parking place that we should check
        if (!isTrackingRoute) {
            // no route tracking, no need to update free parking places information
            return;
        }
        var isCheckInProgress = false;
        setInterval(function() {
            if (!isCheckInProgress) {
                isCheckInProgress = true;
                // find free parkings closest to the target one
                JsApp.findFreeParkings(lat, lon, function() {
                    isCheckInProgress = false;
                }, true);
            }
        }, UPDATE_FREE_PARKING_PLACES_INTERVAL_SECS * 1000);
    }

    return {
        onLocationUpdate: function() {
            determineUserLocation(function() {
                if (isTrackingRoute) {
                    mapInstance.panTo(userLocation);
                }
            });
        },
        onParkingPlaceImageLoaded: function() {
            if (shownParkPlacesCluster) {
                var elementImage = document.getElementById('parking-place-image');
                if (elementImage) {
                    var elementDiv = document.getElementById('parking-place-info-div');
                    var wDiv = elementDiv ? elementDiv.offsetWidth : 0;

                    // fit cluster balloon to picture's size
                    var w = Math.max(elementImage.offsetWidth, wDiv);
                    var h = elementImage.offsetHeight + BALLOON_PARKING_PLACE_TEXT_HEIGHT;

                    var curW = shownParkPlacesCluster.options.get('clusterBalloonContentLayoutWidth');
                    var curH = shownParkPlacesCluster.options.get('clusterBalloonContentLayoutHeight');

                    // if current balloon width/height is more than image's one for only 10 pix, then don't expand the balloon
                    if (curW < w || (curW - w) > 10)
                        shownParkPlacesCluster.options.set('clusterBalloonContentLayoutWidth', w);

                    if (curH < h || (curH - h) > 10)
                        shownParkPlacesCluster.options.set('clusterBalloonContentLayoutHeight', h);
                }
            }
        },
        trackRoute: function(lat, lon) {
            if (mapInstance.balloon.isOpen()) {
                mapInstance.balloon.close();
            }
            isTrackingRoute = true;
            zoomToUserLocation();
            runPeriodicFreeParkingPlaceCheck(lat, lon);
            JavaJsCommunicator.onRouteTrackingStarted();
        },

        buildRoute: function(lat, lon, isAutoRebuild) {
            if (!isTrackingRoute || !isAutoRebuild) {
                showMessageBalloon(lat, lon, 'Прокладываем маршрут...');
            } else {
                log('Прокладываем маршрут...');
            }

            if (userLocation) {
                buildRouteFromTo(userLocation, lat, lon, isAutoRebuild);
                return;
            }

            determineUserLocation(function() {
                buildRouteFromTo(userLocation, lat, lon, isAutoRebuild);
            },
            function(error) {
                if (!isTrackingRoute || !isAutoRebuild) {
                    showMessageBalloon(lat, lon, 'Ошибка определения местоположения: ' + error.message);
                } else {
                    log('Ошибка определения местоположения: ' + error.message);
                }
            });
        },

        findFreeParkings: function(lat, lon, postProcessCallback, isPeriodicCheck) {
            // do not show message boxes in tracking mode if auto-recheck the free parking place
            if (!isTrackingRoute || !isPeriodicCheck) {
                showMessageBalloon(lat, lon, 'Ищем парковки...');
            } else {
                log('Ищем парковки...');
            }

            httpGetAsync(
                FREEPARK_SERVER + '/find-nearest-free-parking-places/?boundary=' +
                    FIND_PARKINGS_NEAR_BOUNDARY_METERS + '&latitude=' + lat + '&longitude=' + lon,
                function(successResult) {
                    var parkingPlaces = JSON.parse(successResult);

                    if (parkingPlaces.length <= 0) {
                        if (!isTrackingRoute || !isPeriodicCheck) {
                            showMessageBalloon(lat, lon, 'Свободных парковок не найдено!');
                        } else {
                            log('Свободных парковок не найдено!');
                        }
                    } else {
                        onFreeParkingPlacesFound(lat, lon, parkingPlaces);
                    }
                    if (postProcessCallback) {
                        postProcessCallback();
                    }
                },
                function(errorResult) {
                    if (!isTrackingRoute || !isPeriodicCheck) {
                        showMessageBalloon(lat, lon, 'Ошибка сервера:' + errorResult);
                    } else {
                        log('server error: ' + errorResult);
                    }
                    if (postProcessCallback) {
                        postProcessCallback();
                    }
                }
            );
        },

        initialize: function(mapElementId, logElementId) {
            logElement = document.getElementById(logElementId);
            createMap(mapElementId);
            showCurrentTraffic();
            zoomToUserLocationAtStart();
            JavaJsCommunicator.onMapInitialized();
        }
    };
}();

ymaps.ready(function() {
    JsApp.initialize('map_div', 'log_div');
});
