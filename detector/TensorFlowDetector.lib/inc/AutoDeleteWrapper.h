#ifndef AutoDeleteWrapper_H
#define AutoDeleteWrapper_H

/**
 * Wraps single TF* pointers in order to auto-remove them on resource clearing.
 */
template<typename T>
class AutoDeleteWrapper {
public:
	typedef void (*DeleteFunc)(T*);

public:
	AutoDeleteWrapper() : m_ptr(nullptr), m_deleteFunc(nullptr) {}
	AutoDeleteWrapper(T * _ptr, DeleteFunc _deleteFunc) : m_ptr(_ptr), m_deleteFunc(_deleteFunc) {}
	AutoDeleteWrapper(AutoDeleteWrapper && _other) : AutoDeleteWrapper(_other.m_ptr, _other.m_deleteFunc) {
		_other.m_ptr = nullptr;
		_other.m_deleteFunc = nullptr;
	}

	AutoDeleteWrapper & operator= (AutoDeleteWrapper && _other) {
		if (this != &_other) {
			m_ptr = _other.m_ptr;
			m_deleteFunc = _other.m_deleteFunc;
			_other.m_ptr = nullptr;
			_other.m_deleteFunc = nullptr;
		}
		return *this;
	}

	AutoDeleteWrapper(const AutoDeleteWrapper & _other) = delete;
	AutoDeleteWrapper & operator= (const AutoDeleteWrapper & _other) = delete;

	~AutoDeleteWrapper() {
		if (m_ptr && m_deleteFunc)
			m_deleteFunc(m_ptr);
	}

	T * get() {return m_ptr;}
	const T * get() const {return m_ptr;}

private:
	T * m_ptr;
	DeleteFunc m_deleteFunc;
};

#endif // AutoDeleteWrapper_H
