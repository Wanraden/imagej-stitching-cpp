/*
 * #%L
 * Fiji distribution of ImageJ for the life sciences.
 * %%
 * Copyright (C) 2007 - 2022 Fiji developers.
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */
// package mpicbg.stitching.fusion;

class PixelFusion
{
public:
	/**
	 *  reset for the next output pixel
	 */
	virtual void clear() = 0;

	/**
	 * add a new value for the current output pixel
	 *
	 * @param value - the image intensity
	 * @param imageId - from which input image as defined by the id
	 * @param localPosition - the position inside the input image in local coordinates of the input image
	 */
	virtual void addValue(double value, int imageId, double localPosition[]) = 0;

	/**
	 *  return the result for the current pixel
	 *
	 * @return - the value for the output image
	 */
	virtual double getValue() = 0;

	/**
	 * Convinience method for multi-threading
	 *
	 * @return - a {@link PixelFusion} with the same properties
	 */
	virtual PixelFusion* copy() = 0;
};
