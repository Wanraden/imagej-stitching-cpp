#pragma once

/**
 * The <code>Transparency</code> interface defines the common transparency
 * modes for implementing classes.
 */
class Transparency {
public:
    /**
     * Represents image data that is guaranteed to be completely opaque,
     * meaning that all pixels have an alpha value of 1.0.
     */
    static int OPAQUE;

    /**
     * Represents image data that is guaranteed to be either completely
     * opaque, with an alpha value of 1.0, or completely transparent,
     * with an alpha value of 0.0.
     */
    static int BITMASK;

    /**
     * Represents image data that contains or might contain arbitrary
     * alpha values between and including 0.0 and 1.0.
     */
    static int TRANSLUCENT;

public:
    /**
     * Returns the type of this <code>Transparency</code>.
     * @return the field type of this <code>Transparency</code>, which is
     *          either OPAQUE, BITMASK or TRANSLUCENT.
     */
    virtual int getTransparency() = 0;
};

int Transparency::OPAQUE = 1;
int Transparency::BITMASK = 2;
int Transparency::TRANSLUCENT = 3;