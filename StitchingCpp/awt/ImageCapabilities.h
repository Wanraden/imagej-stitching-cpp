#pragma once

/*
 * Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//package java.awt;

/**
 * Capabilities and properties of images.
 * @author Michael Martak
 * @since 1.4
 */
class ImageCapabilities {

private:
    bool accelerated = false;

public:
    /**
     * Creates a new object for specifying image capabilities.
     * @param accelerated whether or not an accelerated image is desired
     */
    ImageCapabilities(bool accelerated) {
        this->accelerated = accelerated;
    }

    /**
     * Returns <code>true</code> if the object whose capabilities are
     * encapsulated in this <code>ImageCapabilities</code> can be or is
     * accelerated.
     * @return whether or not an image can be, or is, accelerated.  There are
     * various platform-specific ways to accelerate an image, including
     * pixmaps, VRAM, AGP.  This is the general acceleration method (as
     * opposed to residing in system memory).
     */
    bool isAccelerated() {
        return accelerated;
    }

    /**
     * Returns <code>true</code> if the <code>VolatileImage</code>
     * described by this <code>ImageCapabilities</code> can lose
     * its surfaces.
     * @return whether or not a volatile image is subject to losing its surfaces
     * at the whim of the operating system.
     */
    bool isTrueVolatile() {
        return false;
    }

    /**
     * @return a copy of this ImageCapabilities object.
     */
    ImageCapabilities clone() {
        return *this;
    }
};