# New ports collection makefile for:	lockfree-malloc
# Date created:		2012-08-01
# Whom:			Gvozdikov Veniamin <g.veniamin@googlemail.com>
#
# $FreeBSD$
#

PORTNAME=	lockfree-malloc
PORTVERSION=	0.0.${DATE}
CATEGORIES=	devel
MASTER_SITES=	http://fbsd.zlonet.ru/distfiles/

MAINTAINER=	g.veniamin@googlemail.com
COMMENT=	The world is first Web-scale memory allocator

LICENSE=	LGPL3

LIB_DEPENDS=	boost_system:${PORTSDIR}/devel/boost-libs

DATE=		20120802
ONLY_FOR_ARCHS=	amd64
USE_LDCONFIG=	yes
USE_BZIP2=	yes

OPTIONS=	DOCS	"Install README." on

PLIST_FILES=	lib/liblite-malloc-shared.so \
		lib/liblite-malloc-static.a

.include <bsd.port.pre.mk>

.if defined(WITH_DOCS)
PLIST_FILES+=	share/doc/${PORTNAME}/README
PLIST_DIRS+=	share/doc/${PORTNAME}
.endif

do-install:
.for i in shared.so static.a
	${INSTALL_LIB} ${WRKSRC}/liblite-malloc-${i} ${PREFIX}/lib/liblite-malloc-${i}
.endfor
post-install:
.if defined(WITH_DOCS)
	${MKDIR} ${DOCSDIR}
	cd ${WRKSRC} && ${INSTALL_DATA} README ${DOCSDIR}
.endif

.include <bsd.port.post.mk>
