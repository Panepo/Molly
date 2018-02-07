#pragma once

#include <opencv2\core\core.hpp>

#ifndef MOLIRISI_CONFIG_H
#define MOLIRISI_CONFIG_H

#define EnableColorStream		0

#if EnableColorStream
#define ColorStreamResWidth		1280
#define ColorStreamResHeight	720
#define ColorStreamFPS			30
#endif // EnableColorStream

#define EnableInfraredStream	1

#if EnableInfraredStream
#define InfraredStreamResWidth	1280
#define InfraredStreamResHeight	720
#define InfraredStreamFPS		30
#endif // EnableInfraredStream

#define EnableDepthStream		1

#if EnableDepthStream
#define	DepthStreamResWidth		1280
#define DepthStreamResHeight	720
#define DepthStreamFPS			30
#endif // EnableDepthStream


#endif // MOLIRISI_CONFIG_H