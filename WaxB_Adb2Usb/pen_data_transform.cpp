/*
 * pen_data_transform.cpp
 *
 *  Created on: 2011-08-09
 *      Author: Bernard
 */

#include <stdint.h>

#include "console.h"
#include "extdata.h"
#include "pen_data_transform.h"

/** Module that converts x/y coordinates and all other pen data between the native tablet and the usb tablet.
 *  The tablet rotation setting is taken into account to "rotate" everything accordingly. (e.g. x, y, tilt, z-rotation, etc.)
 *
 * PenDataTransform::init() needs to be called at startup
 */
namespace PenDataTransform
{
	static uint16_t	slaveXMin;
	static uint16_t	slaveXMax;
	static uint16_t	slaveYMin;
	static uint16_t	slaveYMax;
	static uint16_t	usbXMin;
	static uint16_t	usbXMax;
	static uint16_t	usbYMin;
	static uint16_t	usbYMax;

	static bool hasXAnchor;
	static bool hasYAnchor;
	static uint16_t	slaveXAnchorMin;
	static uint16_t	slaveXAnchorMax;
	static uint16_t	slaveYAnchorMin;
	static uint16_t	slaveYAnchorMax;
	static uint16_t	usbXAnchorMin;
	static uint16_t	usbXAnchorMax;
	static uint16_t	usbYAnchorMin;
	static uint16_t	usbYAnchorMax;

	static uint16_t usbPressureMax;
	static uint16_t	slavePressureMax;

	void init()
	{
		slaveXMin = extdata_getValue16(EXTDATA_SLAVE_X_MIN);
		slaveXMax = extdata_getValue16(EXTDATA_SLAVE_X_MAX);
		slaveYMin = extdata_getValue16(EXTDATA_SLAVE_Y_MIN);
		slaveYMax = extdata_getValue16(EXTDATA_SLAVE_Y_MAX);

		console::print(" slaveXMax:");
		console::printNumber(slaveXMax);
		console::print(" slaveYMax:");
		console::printNumber(slaveYMax);

		usbXMin = extdata_getValue16(EXTDATA_USB_X_MIN);
		usbXMax = extdata_getValue16(EXTDATA_USB_X_MAX);
		usbYMin = extdata_getValue16(EXTDATA_USB_Y_MIN);
		usbYMax = extdata_getValue16(EXTDATA_USB_Y_MAX);

		console::print(" usbXMax:");
		console::printNumber(usbXMax);
		console::print(" usbYMax:");
		console::printNumber(usbYMax);
		console::print("\n");

		slaveXAnchorMin = extdata_getValue16(EXTDATA_SLAVE_X_ANCHOR_MIN);
		slaveXAnchorMax = extdata_getValue16(EXTDATA_SLAVE_X_ANCHOR_MAX);
		slaveYAnchorMin = extdata_getValue16(EXTDATA_SLAVE_Y_ANCHOR_MIN);
		slaveYAnchorMax = extdata_getValue16(EXTDATA_SLAVE_Y_ANCHOR_MAX);

		usbXAnchorMin = extdata_getValue16(EXTDATA_USB_X_ANCHOR_MIN);
		usbXAnchorMax = extdata_getValue16(EXTDATA_USB_X_ANCHOR_MAX);
		usbYAnchorMin = extdata_getValue16(EXTDATA_USB_Y_ANCHOR_MIN);
		usbYAnchorMax = extdata_getValue16(EXTDATA_USB_Y_ANCHOR_MAX);

		slavePressureMax = extdata_getValue16(EXTDATA_SLAVE_PRESSURE_MAX);
		usbPressureMax = extdata_getValue16(EXTDATA_USB_PRESSURE_MAX);

		hasXAnchor = !(	slaveXAnchorMin == 0 && slaveXAnchorMax == 0 &&
						usbXAnchorMin == 0 && usbXAnchorMax == 0);

		hasYAnchor = !(	slaveYAnchorMin == 0 && slaveYAnchorMax == 0 &&
						usbYAnchorMin == 0 && usbYAnchorMax == 0);


		if(hasXAnchor)
		{
			console::print("has XAnchor\n");
			if(slaveXAnchorMax == 0)
				slaveXAnchorMax = slaveXMax;
			else if(slaveXAnchorMax > slaveXMax)
				slaveXAnchorMax = slaveXMax;

			if(slaveXAnchorMin == 0)
				slaveXAnchorMin = slaveXMin;
			else if(slaveXAnchorMin < slaveXMin)
				slaveXAnchorMin = slaveXMin;
		}

		if(hasYAnchor)
		{
			console::print("has YAnchor\n");
			if(slaveYAnchorMax == 0)
				slaveYAnchorMax = slaveYMax;
			else if(slaveYAnchorMax > slaveYMax)
				slaveYAnchorMax = slaveYMax;

			if(slaveYAnchorMin == 0)
				slaveYAnchorMin = slaveYMin;
			else if(slaveYAnchorMin < slaveYMin)
				slaveYAnchorMin = slaveYMin;
		}
	}

	uint16_t compute_position(uint16_t sourcevalue,
					uint16_t minsource, uint16_t maxsource,
					uint16_t mintarget, uint16_t maxtarget)
	{
/*		console::print("compute_position(");
		console::printNumber(sourcevalue);
		console::print(",");
		console::printNumber(minsource);
		console::print(",");
		console::printNumber(maxsource);
		console::print(",");
		console::printNumber(mintarget);
		console::print(",");
		console::printNumber(maxtarget);
		console::print(")\n");
*/

		// crop big values to the "max" value
		if(sourcevalue >= maxsource)
			return maxtarget;

		uint16_t norm_sourcevalue = sourcevalue - minsource;

		// crop small values the "min" value
		if(norm_sourcevalue <= 0)
			return mintarget;

		uint16_t norm_maxtarget = maxtarget - mintarget;

		uint32_t val32 = ((uint32_t)norm_sourcevalue) * norm_maxtarget;

		return ((uint16_t)(val32 / maxsource)) + mintarget;
	}

	uint16_t compute_x_position(uint16_t sourcevalue)
	{
		if(hasXAnchor)
		{
			if(sourcevalue >= slaveXAnchorMin)
			{
				if(sourcevalue <= slaveXAnchorMax)
				{
					// main (central) area
					return compute_position(sourcevalue,
						slaveXAnchorMin, slaveXAnchorMax,
						usbXAnchorMin, usbXAnchorMax);
				}
				else
				{
					// max area
					return compute_position(sourcevalue,
						slaveXAnchorMax, slaveXMax,
						usbXAnchorMax, usbXMax);
				}
			}
			else
			{
				// min area
				return compute_position(sourcevalue,
						slaveXMin, slaveXAnchorMin,
						usbXMin, usbXAnchorMin);
			}
		}
		else
		{
			// no anchor processing
			return compute_position(sourcevalue,
					slaveXMin, slaveXMax,
					usbXMin, usbXMax);
		}
	}

	uint16_t compute_y_position(uint16_t sourcevalue)
	{
		if(hasYAnchor)
		{
			if(sourcevalue >= slaveYAnchorMin)
			{
				if(sourcevalue <= slaveYAnchorMax)
				{
					// main (central) area
					return compute_position(sourcevalue,
						slaveYAnchorMin, slaveYAnchorMax,
						usbYAnchorMin, usbYAnchorMax);
				}
				else
				{
					// max area
					return compute_position(sourcevalue,
						slaveYAnchorMax, slaveYMax,
						usbYAnchorMax, usbYMax);
				}
			}
			else
			{
				// min area
				return compute_position(sourcevalue,
						slaveYMin, slaveYAnchorMin,
						usbYMin, usbYAnchorMin);
			}
		}
		else
		{
			// no anchor processing
			return compute_position(sourcevalue,
					slaveYMin, slaveYMax,
					usbYMin, usbYMax);
		}
	}

	inline uint16_t scale_255_to_511(uint16_t source)
	{
		// this normally cannot get to 511, only 510. To get to 511 we need to jump a value.
		// using standard maths would give us a "break" near the middle which might not be desirable
		// so the solution to support 511, is to make 510 (or source=255) clamp to 511:

//		if(source < 255)
			return source << 1;
//		else
//			return 511;
	}

	uint16_t scale_uint16(uint16_t sourcevalue, uint16_t maxsource, uint16_t maxtarget)
	{
		if(maxsource == 0)
			return 0;

		uint32_t val32 = ((uint32_t)sourcevalue) * maxtarget;

		return (uint16_t)(val32 / maxsource);
	}

	uint16_t compute_pressure(uint16_t sourcevalue)
	{
/*		console::print("compute pressure(");
		console::printNumber(sourcevalue);
		console::print(",");
		console::printNumber(slavePressureMax);
		console::print(",");
		console::printNumber(usbPressureMax);
		console::print(")\n");
*/
		if(sourcevalue == slavePressureMax)
		{
			// make sure we get to 100% pressure
			return usbPressureMax;
		}

		if(slavePressureMax == 255 && usbPressureMax == 511)
		{
			// this normally cannot get to 511, only 510. To get to 511 we need to jump a value.
			// using standard maths would give us a "break" near the middle which might not be desirable.
			//
			// so the solution is to compute max=510 (clamp to 511 when = 510, which is done in the
			// '100%' step above):
			//
			// Note: this case could also apply to mappings like 255 -> 1023 but the aliasing is not
			// as "bad".

			return sourcevalue << 1;
		}

//		console::print("...generic scaling\n");

		// generic scaling
		return scale_uint16(sourcevalue, slavePressureMax, usbPressureMax);
	}

	void transform_pen_data(Pen::PenEvent& penEvent, PenDataTransform::XformedData& xformed, bool withtilt)
	{
		xformed.x = compute_x_position(penEvent.x);
		xformed.y = compute_y_position(penEvent.y);
		xformed.pressure = compute_pressure(penEvent.pressure);

/*		console::print("\npenEvent.x = ");
		console::printNumber(penEvent.x);
		console::print(", xformed.x = ");
		console::printNumber(xformed.x);
		console::println();
*/
		if(withtilt)
		{
			xformed.tilt_x = penEvent.tilt_x;
			xformed.tilt_y = penEvent.tilt_y;
		}
	}
}
