#pragma once

#ifdef __cplusplus
#include <QObject>

class StrPtr : public QObject {
	Q_OBJECT
public:
	void *p;
	size_t np;
	StrPtr();
	StrPtr(void *ptr, size_t len);
	StrPtr(const StrPtr& s);
	~StrPtr() {};
};
Q_DECLARE_METATYPE(StrPtr);

class CoreThread : public QObject {
	Q_OBJECT
public:        CoreThread();
public slots:  void thr_started();
signals:
	void draw_base(StrPtr p);
	void draw_img(StrPtr p);
	void enable_im();
	void disable_im();
};

#endif

