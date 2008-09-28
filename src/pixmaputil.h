#ifndef PIXMAPUTIL_H
#define PIXMAPUTIL_H

class QPixmap;

namespace PixmapUtil
{
	QPixmap createTransparentPixmap(int width, int height);
	QPixmap rotatedPixmap(int rotation, const QPixmap& sourcePixmap);
};

#endif
