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
//import java.util.ArrayList;
#include "PixelFusion.h"

class BlendingPixelFusion : PixelFusion
{
public:
	double fractionBlended = 0.2;
	
	int numDimensions;
	int numImages;
	long **dimensions;
	double percentScaling;
	double *border;
	
	ArrayList< ? : ImageInterpolation< ? > > images;

	double valueSum, weightSum;
	
public:
	/**
	 * Instantiates the per-pixel blending
	 * 
	 * @param images - all input images (the position in the list has to be the same as Id provided by addValue!)
	 */
	BlendingPixelFusion( ArrayList< ? : ImageInterpolation< ? > > images )
	{
		BlendingPixelFusion( images, fractionBlended );
	}	
private:
	/**
	 * Instantiates the per-pixel blending
	 * 
	 * @param images - all input images (the position in the list has to be the same as Id provided by addValue!)
	 * @param percentScaling - which percentage of the image should be blended ( e.g. 0,3 means 15% on the left and 15% on the right)
	 */
	BlendingPixelFusion( ArrayList< ? : ImageInterpolation< ? > > images, double fractionBlended )
	{
		this->images = images;
		this->percentScaling = fractionBlended;
		
		this->numDimensions = images.get( 0 ).getImg().numDimensions();
		this->numImages = images.size();
		this->dimensions = new long[ numImages ][ numDimensions ];
		
		for ( int i = 0; i < numImages; ++i )
			for ( int d = 0; d < numDimensions; ++d )
				dimensions[ i ][ d ] = images.get( i ).getImg().dimension( d ) - 1; 

		this->border = new double[ numDimensions ];

		// reset
		clear();
	}
	
	
	virtual void clear() override { valueSum = weightSum = 0; }

	
	virtual void addValue( double value, int imageId, double localPosition[]) override
	{
		// we are always inside the image, so we do not want 0.0
		double weight = max( 0.00001, computeWeight( localPosition, dimensions[ imageId ], border, percentScaling ) );
		
		weightSum += weight;
		valueSum += value * weight;
	}

	
	virtual double getValue() override
	{ 
		if ( weightSum == 0 )
			return 0;
		return ( valueSum / weightSum );
	}

	
	virtual PixelFusion* copy() { return new BlendingPixelFusion( images ); }

	/**
	 * From SPIM Registration
	 * 
	 * @param location
	 * @param dimensions
	 * @param border
	 * @param percentScaling
	 * @return
	 */
	static double computeWeight( double[] location, long[] dimensions, double[] border, double percentScaling )
	{		
		// compute multiplicative distance to the respective borders [0...1]
		double minDistance = 1;
		
		for ( int dim = 0; dim < location.length; ++dim )
		{
			// the position in the image
			double localImgPos = location[ dim ];
			
			// the distance to the border that is closer
			double value = Math.max( 1, Math.min( localImgPos - border[ dim ] + 1, (dimensions[ dim ] - 1) - localImgPos - border[ dim ] + 1 ) );
						
			float imgAreaBlend = Math.round( percentScaling * 0.5f * dimensions[ dim ] );
			
			if ( value < imgAreaBlend )
				value = value / imgAreaBlend;
			else
				value = 1;
			
			minDistance *= value;
		}
		
		if ( minDistance == 1 )
			return 1;
		else if ( minDistance <= 0 )
			return 0.0000001;
		else
			return ( Math.cos( (1 - minDistance) * Math.PI ) + 1 ) / 2;				
	}

};
