#pragma once

#include "header.h"

class ColorSpace {

	long serialVersionUID = -409452704308689724L;

private:
	int type;
	int numComponents;
	vector<string> compName;

	// Cache of singletons for the predefined color spaces.
	static ColorSpace sRGBspace;
	static ColorSpace XYZspace;
	static ColorSpace PYCCspace;
	static ColorSpace GRAYspace;
	static ColorSpace LINEAR_RGBspace;

public:

	/**
	 * Any of the family of XYZ color spaces.
	 */
	static const int TYPE_XYZ = 0;

	/**
	 * Any of the family of Lab color spaces.
	 */
	static const int TYPE_Lab = 1;

	/**
	 * Any of the family of Luv color spaces.
	 */
	static const int TYPE_Luv = 2;

	/**
	 * Any of the family of YCbCr color spaces.
	 */
	static const int TYPE_YCbCr = 3;

	/**
	 * Any of the family of Yxy color spaces.
	 */
	static const int TYPE_Yxy = 4;

	/**
	 * Any of the family of RGB color spaces.
	 */
	static const int TYPE_RGB = 5;

	/**
	 * Any of the family of GRAY color spaces.
	 */
	static const int TYPE_GRAY = 6;

	/**
	 * Any of the family of HSV color spaces.
	 */
	static const int TYPE_HSV = 7;

	/**
	 * Any of the family of HLS color spaces.
	 */
	static const int TYPE_HLS = 8;

	/**
	 * Any of the family of CMYK color spaces.
	 */
	static const int TYPE_CMYK = 9;

	/**
	 * Any of the family of CMY color spaces.
	 */
	static const int TYPE_CMY = 11;

	/**
	 * Generic 2 component color spaces.
	 */
	static const int TYPE_2CLR = 12;

	/**
	 * Generic 3 component color spaces.
	 */
	static const int TYPE_3CLR = 13;

	/**
	 * Generic 4 component color spaces.
	 */
	static const int TYPE_4CLR = 14;

	/**
	 * Generic 5 component color spaces.
	 */
	static const int TYPE_5CLR = 15;

	/**
	 * Generic 6 component color spaces.
	 */
	static const int TYPE_6CLR = 16;

	/**
	 * Generic 7 component color spaces.
	 */
	static const int TYPE_7CLR = 17;

	/**
	 * Generic 8 component color spaces.
	 */
	static const int TYPE_8CLR = 18;

	/**
	 * Generic 9 component color spaces.
	 */
	static const int TYPE_9CLR = 19;

	/**
	 * Generic 10 component color spaces.
	 */
	static const int TYPE_ACLR = 20;

	/**
	 * Generic 11 component color spaces.
	 */
	static const int TYPE_BCLR = 21;

	/**
	 * Generic 12 component color spaces.
	 */
	static const int TYPE_CCLR = 22;

	/**
	 * Generic 13 component color spaces.
	 */
	static const int TYPE_DCLR = 23;

	/**
	 * Generic 14 component color spaces.
	 */
	static const int TYPE_ECLR = 24;

	/**
	 * Generic 15 component color spaces.
	 */
	static const int TYPE_FCLR = 25;

	/**
	 * The sRGB color space defined at
	 * <A href="http://www.w3.org/pub/WWW/Graphics/Color/sRGB.html">
	 * http://www.w3.org/pub/WWW/Graphics/Color/sRGB.html
	 * </A>.
	 */
	static const int CS_sRGB = 1000;

	/**
	 * A built-in linear RGB color space.  This space is based on the
	 * same RGB primaries as CS_sRGB, but has a linear tone reproduction curve.
	 */
	static const int CS_LINEAR_RGB = 1004;

	/**
	 * The CIEXYZ conversion color space defined above.
	 */
	static const int CS_CIEXYZ = 1001;

	/**
	 * The Photo YCC conversion color space.
	 */
	static const int CS_PYCC = 1002;

	/**
	 * The built-in linear gray scale color space.
	 */
	static const int CS_GRAY = 1003;



protected:
	/**
	 * Constructs a ColorSpace object given a color space type
	 * and the number of components.
	 * @param type one of the <CODE>ColorSpace</CODE> type constants
	 * @param numcomponents the number of components in the color space
	 */
	ColorSpace(int type, int numcomponents) {
		this->type = type;
		this->numComponents = numcomponents;
	}


	/**
	 * Returns a ColorSpace representing one of the specific
	 * predefined color spaces.
	 * @param colorspace a specific color space identified by one of
	 *        the predefined class constants (e.g. CS_sRGB, CS_LINEAR_RGB,
	 *        CS_CIEXYZ, CS_GRAY, or CS_PYCC)
	 * @return the requested <CODE>ColorSpace</CODE> object
	 */
	 // NOTE: This method may be called by privileged threads.
	 //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
public:
	static ColorSpace* getInstance(int colorspace)
	{
		ColorSpace* theColorSpace = nullptr;
		switch (colorspace) {
		case CS_sRGB:
			synchronized(ColorSpace::class) {
				if (sRGBspace == null) {
					ICC_Profile theProfile = ICC_Profile.getInstance(CS_sRGB);
					sRGBspace = new ICC_ColorSpace(theProfile);
				}

				theColorSpace = &sRGBspace;
			}
			break;

		case CS_CIEXYZ:
			synchronized(ColorSpace::class) {
				if (XYZspace == null) {
					ICC_Profile theProfile =
						ICC_Profile.getInstance(CS_CIEXYZ);
					XYZspace = new ICC_ColorSpace(theProfile);
				}

				theColorSpace = XYZspace;
			}
			break;

		case CS_PYCC:
			synchronized(ColorSpace::class) {
				if (PYCCspace == null) {
					ICC_Profile theProfile = ICC_Profile.getInstance(CS_PYCC);
					PYCCspace = new ICC_ColorSpace(theProfile);
				}

				theColorSpace = PYCCspace;
			}
			break;


		case CS_GRAY:
			synchronized(ColorSpace::class) {
				if (GRAYspace == null) {
					ICC_Profile theProfile = ICC_Profile.getInstance(CS_GRAY);
					GRAYspace = new ICC_ColorSpace(theProfile);
					/* to allow access from java.awt.ColorModel */
					CMSManager.GRAYspace = GRAYspace;
				}

				theColorSpace = GRAYspace;
			}
			break;


		case CS_LINEAR_RGB:
			synchronized(ColorSpace::class) {
				if (LINEAR_RGBspace == null) {
					ICC_Profile theProfile =
						ICC_Profile.getInstance(CS_LINEAR_RGB);
					LINEAR_RGBspace = new ICC_ColorSpace(theProfile);
					/* to allow access from java.awt.ColorModel */
					CMSManager.LINEAR_RGBspace = LINEAR_RGBspace;
				}

				theColorSpace = LINEAR_RGBspace;
			}
			break;


		default:
			throw ("Unknown color space");
		}

		return theColorSpace;
	}

public:
	/**
	 * Returns true if the ColorSpace is CS_sRGB.
	 * @return <CODE>true</CODE> if this is a <CODE>CS_sRGB</CODE> color
	 *         space, <code>false</code> if it is not
	 */
	bool isCS_sRGB() {
		/* REMIND - make sure we know sRGBspace exists already */
		return (this == &sRGBspace);
	}

	/**
	 * Transforms a color value assumed to be in this ColorSpace
	 * into a value in the default CS_sRGB color space.
	 * <p>
	 * This method transforms color values using algorithms designed
	 * to produce the best perceptual match between input and output
	 * colors.  In order to do colorimetric conversion of color values,
	 * you should use the <code>toCIEXYZ</code>
	 * method of this color space to first convert from the input
	 * color space to the CS_CIEXYZ color space, and then use the
	 * <code>fromCIEXYZ</code> method of the CS_sRGB color space to
	 * convert from CS_CIEXYZ to the output color space.
	 * See {@link #toCIEXYZ(float[]) toCIEXYZ} and
	 * {@link #fromCIEXYZ(float[]) fromCIEXYZ} for further information.
	 * <p>
	 * @param colorvalue a float array with length of at least the number
	 *        of components in this ColorSpace
	 * @return a float array of length 3
	 * @throws ArrayIndexOutOfBoundsException if array length is not
	 *         at least the number of components in this ColorSpace
	 */
	virtual float* toRGB(float colorvalue[]) = 0;


	/**
	 * Transforms a color value assumed to be in the default CS_sRGB
	 * color space into this ColorSpace::
	 * <p>
	 * This method transforms color values using algorithms designed
	 * to produce the best perceptual match between input and output
	 * colors.  In order to do colorimetric conversion of color values,
	 * you should use the <code>toCIEXYZ</code>
	 * method of the CS_sRGB color space to first convert from the input
	 * color space to the CS_CIEXYZ color space, and then use the
	 * <code>fromCIEXYZ</code> method of this color space to
	 * convert from CS_CIEXYZ to the output color space.
	 * See {@link #toCIEXYZ(float[]) toCIEXYZ} and
	 * {@link #fromCIEXYZ(float[]) fromCIEXYZ} for further information.
	 * <p>
	 * @param rgbvalue a float array with length of at least 3
	 * @return a float array with length equal to the number of
	 *         components in this ColorSpace
	 * @throws ArrayIndexOutOfBoundsException if array length is not
	 *         at least 3
	 */
	virtual float* fromRGB(float rgbvalue[]) = 0;


	/**
	 * Transforms a color value assumed to be in this ColorSpace
	 * into the CS_CIEXYZ conversion color space.
	 * <p>
	 * This method transforms color values using relative colorimetry,
	 * as defined by the International Color Consortium standard.  This
	 * means that the XYZ values returned by this method are represented
	 * relative to the D50 white point of the CS_CIEXYZ color space.
	 * This representation is useful in a two-step color conversion
	 * process in which colors are transformed from an input color
	 * space to CS_CIEXYZ and then to an output color space.  This
	 * representation is not the same as the XYZ values that would
	 * be measured from the given color value by a colorimeter.
	 * A further transformation is necessary to compute the XYZ values
	 * that would be measured using current CIE recommended practices.
	 * See the {@link ICC_ColorSpace#toCIEXYZ(float[]) toCIEXYZ} method of
	 * <code>ICC_ColorSpace</code> for further information.
	 * <p>
	 * @param colorvalue a float array with length of at least the number
	 *        of components in this ColorSpace
	 * @return a float array of length 3
	 * @throws ArrayIndexOutOfBoundsException if array length is not
	 *         at least the number of components in this ColorSpace::
	 */
	virtual float* toCIEXYZ(float colorvalue[]) = 0;


	/**
	 * Transforms a color value assumed to be in the CS_CIEXYZ conversion
	 * color space into this ColorSpace::
	 * <p>
	 * This method transforms color values using relative colorimetry,
	 * as defined by the International Color Consortium standard.  This
	 * means that the XYZ argument values taken by this method are represented
	 * relative to the D50 white point of the CS_CIEXYZ color space.
	 * This representation is useful in a two-step color conversion
	 * process in which colors are transformed from an input color
	 * space to CS_CIEXYZ and then to an output color space.  The color
	 * values returned by this method are not those that would produce
	 * the XYZ value passed to the method when measured by a colorimeter.
	 * If you have XYZ values corresponding to measurements made using
	 * current CIE recommended practices, they must be converted to D50
	 * relative values before being passed to this method.
	 * See the {@link ICC_ColorSpace#fromCIEXYZ(float[]) fromCIEXYZ} method of
	 * <code>ICC_ColorSpace</code> for further information.
	 * <p>
	 * @param colorvalue a float array with length of at least 3
	 * @return a float array with length equal to the number of
	 *         components in this ColorSpace
	 * @throws ArrayIndexOutOfBoundsException if array length is not
	 *         at least 3
	 */
	virtual float* fromCIEXYZ(float colorvalue[]) = 0;

	/**
	 * Returns the color space type of this ColorSpace (for example
	 * TYPE_RGB, TYPE_XYZ, ...).  The type defines the
	 * number of components of the color space and the interpretation,
	 * e.g. TYPE_RGB identifies a color space with three components - red,
	 * green, and blue.  It does not define the particular color
	 * characteristics of the space, e.g. the chromaticities of the
	 * primaries.
	 *
	 * @return the type constant that represents the type of this
	 *         <CODE>ColorSpace</CODE>
	 */
	int getType() {
		return type;
	}

	/**
	 * Returns the number of components of this ColorSpace::
	 * @return The number of components in this <CODE>ColorSpace</CODE>.
	 */
	int getNumComponents() {
		return numComponents;
	}

	/**
	 * Returns the name of the component given the component index.
	 *
	 * @param idx the component index
	 * @return the name of the component at the specified index
	 * @throws IllegalArgumentException if <code>idx</code> is
	 *         less than 0 or greater than numComponents - 1
	 */
	string getName(int idx) {
		/* REMIND - handle common cases here */
		if ((idx < 0) || (idx > numComponents - 1)) {
			throw "Component index out of range: " + idx;
		}

		if (compName.empty()) {
			switch (type) {
			case ColorSpace::TYPE_XYZ:
				compName = { "X", "Y", "Z" };
				break;
			case ColorSpace::TYPE_Lab:
				compName = { "L", "a", "b" };
				break;
			case ColorSpace::TYPE_Luv:
				compName = { "L", "u", "v" };
				break;
			case ColorSpace::TYPE_YCbCr:
				compName = { "Y", "Cb", "Cr" };
				break;
			case ColorSpace::TYPE_Yxy:
				compName = { "Y", "x", "y" };
				break;
			case ColorSpace::TYPE_RGB:
				compName = { "Red", "Green", "Blue" };
				break;
			case ColorSpace::TYPE_GRAY:
				compName = { "Gray" };
				break;
			case ColorSpace::TYPE_HSV:
				compName = { "Hue", "Saturation", "Value" };
				break;
			case ColorSpace::TYPE_HLS:
				compName = { "Hue", "Lightness", "Saturation" };
				break;
			case ColorSpace::TYPE_CMYK:
				compName = { "Cyan", "Magenta", "Yellow", "Black" };
				break;
			case ColorSpace::TYPE_CMY:
				compName = { "Cyan", "Magenta", "Yellow" };
				break;
			default:
				vector<string> tmp(numComponents);
				for (int i = 0; i < tmp.size(); i++) {
					tmp[i] = "Unnamed color component(" + to_string(i) + ")";
				}
				compName = tmp;
			}
		}
		return compName[idx];
	}

	/**
	 * Returns the minimum normalized color component value for the
	 * specified component.  The default implementation in this abstract
	 * class returns 0.0 for all components.  Subclasses should override
	 * this method if necessary.
	 *
	 * @param component the component index
	 * @return the minimum normalized component value
	 * @throws IllegalArgumentException if component is less than 0 or
	 *         greater than numComponents - 1
	 * @since 1.4
	 */
	float getMinValue(int component) {
		if ((component < 0) || (component > numComponents - 1)) {
			throw "Component index out of range: " + component;
		}
		return 0.0f;
	}

	/**
	 * Returns the maximum normalized color component value for the
	 * specified component.  The default implementation in this abstract
	 * class returns 1.0 for all components.  Subclasses should override
	 * this method if necessary.
	 *
	 * @param component the component index
	 * @return the maximum normalized component value
	 * @throws IllegalArgumentException if component is less than 0 or
	 *         greater than numComponents - 1
	 * @since 1.4
	 */
	float getMaxValue(int component) {
		if ((component < 0) || (component > numComponents - 1)) {
			throw "Component index out of range: " + component;
		}
		return 1.0f;
	}

	/* Returns true if cspace is the XYZspace.
	 */
	static bool isCS_CIEXYZ(ColorSpace* cspace) {
		return (cspace == &XYZspace);
	}
};