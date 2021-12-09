from django.conf.urls import url, include
from rest_framework.routers import DefaultRouter
from rest_framework.schemas import get_schema_view
from rest_framework.authtoken.views import obtain_auth_token
from rest_api import views

schema_view = get_schema_view(title='Freepark backend API')

# Create a router and register our viewsets with it.
router = DefaultRouter()
router.register(r'parkings', views.ParkingViewSet)
router.register(r'cameras', views.CameraViewSet)
router.register(r'free-parking-places', views.FreeParkingPlaceViewSet)

# The API URLs are now determined automatically by the router.
# Additionally, we include the login URLs for the browsable API.
urlpatterns = [
    url(r'^schema/$', schema_view),
    url(r'^', include(router.urls)),
    url(r'^find-nearest-free-parking-places/', views.find_nearest_free_parking_places),
    url(r'^api-auth/', include('rest_framework.urls', namespace='rest_framework')),
    url(r'^get-token/', obtain_auth_token)
]
