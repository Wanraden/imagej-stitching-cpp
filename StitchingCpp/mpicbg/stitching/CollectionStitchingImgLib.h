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
// package mpicbg.stitching;

import ij.IJ;
import ij.gui.Roi;

import java.awt.Rectangle;
import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicInteger;

import stitching.utils.Log;
import mpicbg.imglib.multithreading.SimpleMultiThreading;
import mpicbg.imglib.util.Util;
import mpicbg.models.TranslationModel2D;
import mpicbg.models.TranslationModel3D;

class CollectionStitchingImgLib 
{

	public static ArrayList< ImagePlusTimePoint > stitchCollection( ArrayList< ImageCollectionElement > elements, StitchingParameters params )
	{
		// the result
		ArrayList< ImagePlusTimePoint > optimized;
		
		if ( params.computeOverlap )
		{
			// find overlapping tiles
			Vector< ComparePair > pairs = findOverlappingTiles( elements, params );
			
			if ( pairs == null || pairs.isEmpty())
			{
				LOGERR( "No overlapping tiles could be found given the approximate layout." );
				return null;
			}
			
			// compute all compare pairs
			// compute all matchings
			AtomicInteger ai = new AtomicInteger(0);
			
			int numThreads;
			
			if ( params.cpuMemChoice == 0 )
				numThreads = 1;
			else
				numThreads = Runtime.getRuntime().availableProcessors();
			
	        Thread[] threads = SimpleMultiThreading.newThreads( numThreads );
	    	
	        for ( int ithread = 0; ithread < threads.length; ++ithread )
	            threads[ ithread ] = new Thread(new Runnable()
	            {
	                @Override
	                public void run()
	                {		
	                   	int myNumber = ai.getAndIncrement();
	                    
	                    for ( int i = 0; i < pairs.size(); i++ )
	                    {
	                    	if ( i % numThreads == myNumber )
	                    	{
	                    		ComparePair pair = pairs.get( i );
	                    		
	                    		long start = TimeHelper::milliseconds();			
	                			
	                    		// where do we approximately overlap?
	                			Roi roi1 = getROI( pair.getTile1().getElement(), pair.getTile2().getElement() );
	                			Roi roi2 = getROI( pair.getTile2().getElement(), pair.getTile1().getElement() );
	                			
	            				PairWiseStitchingResult result = PairWiseStitchingImgLib.stitchPairwise( pair.getImagePlus1(), pair.getImagePlus2(), roi1, roi2, pair.getTimePoint1(), pair.getTimePoint2(), params );			
	            				if ( result == null )
	            				{
	            					LOGERR( "Collection stitching failed" );
	            					return;
	            				}
	
	            				if ( params.dimensionality == 2 )
	            					pair.setRelativeShift( new float[]{ result.getOffset( 0 ), result.getOffset( 1 ) } );
	            				else
	            					pair.setRelativeShift( new float[]{ result.getOffset( 0 ), result.getOffset( 1 ), result.getOffset( 2 ) } );
	            				
	            				pair.setCrossCorrelation( result.getCrossCorrelation() );
	
	            				LOGINFO( pair.getImagePlus1().getTitle() + "[" + pair.getTimePoint1() + "]" + " -> " + pair.getImagePlus2().getTitle() + "[" + pair.getTimePoint2() + "]" + ": " +
	            						Util.printCoordinates( result.getOffset() ) + " correlation (R)=" + result.getCrossCorrelation() + " (" + (TimeHelper::milliseconds() - start) + " ms)");
	                    	}
	                    }
	                }
	            });
	        
	        long time = TimeHelper::milliseconds();
	        SimpleMultiThreading.startAndJoin( threads );
	        
	        // get the positions of all tiles
			optimized = GlobalOptimization.optimize( pairs, pairs.get( 0 ).getTile1(), params );
			LOGINFO( "Finished registration process (" + (TimeHelper::milliseconds() - time) + " ms)." );
		}
		else
		{
			// all ImagePlusTimePoints, each of them needs its own model
			optimized = new ArrayList< ImagePlusTimePoint >();
			
			for ( ImageCollectionElement element : elements )
			{
				ImagePlusTimePoint imt = new ImagePlusTimePoint( element.open( params.virtual ), element.getIndex(), 1, element.getModel(), element );
				
				// set the models to the offset
				if ( params.dimensionality == 2 )
				{
					TranslationModel2D model = (TranslationModel2D)imt.getModel();
					model.set( element.getOffset( 0 ), element.getOffset( 1 ) );
				}
				else
				{
					TranslationModel3D model = (TranslationModel3D)imt.getModel();
					model.set( element.getOffset( 0 ), element.getOffset( 1 ), element.getOffset( 2 ) );					
				}
				
				optimized.add( imt );
			}
			
		}
		
		return optimized;
	}

	protected static Roi getROI( ImageCollectionElement e1, ImageCollectionElement e2 )
	{
		int start[] = new int[ 2 ], end[] = new int[ 2 ];
		
		for ( int dim = 0; dim < 2; ++dim )
		{			
			// begin of 2 lies inside 1
			if ( e2.offset[ dim ] >= e1.offset[ dim ] && e2.offset[ dim ] <= e1.offset[ dim ] + e1.size[ dim ] )
			{
				start[ dim ] = Math.round( e2.offset[ dim ] - e1.offset[ dim ] );
				
				// end of 2 lies inside 1
				if ( e2.offset[ dim ] + e2.size[ dim ] <= e1.offset[ dim ] + e1.size[ dim ] )
					end[ dim ] = Math.round( e2.offset[ dim ] + e2.size[ dim ] - e1.offset[ dim ] );
				else
					end[ dim ] = Math.round( e1.size[ dim ] );
			}
			else if ( e2.offset[ dim ] + e2.size[ dim ] <= e1.offset[ dim ] + e1.size[ dim ] ) // end of 2 lies inside 1
			{
				start[ dim ] = 0;
				end[ dim ] = Math.round( e2.offset[ dim ] + e2.size[ dim ] - e1.offset[ dim ] );
			}
			else // if both outside then the whole image 
			{
				start[ dim ] = -1;
				end[ dim ] = -1;
			}
		}
		
		return new Roi( new Rectangle( start[ 0 ], start[ 1 ], end[ 0 ] - start[ 0 ], end[ 1 ] - start[ 1 ] ) );
	}

	protected static Vector< ComparePair > findOverlappingTiles( ArrayList< ImageCollectionElement > elements, StitchingParameters params )
	{		
		for ( ImageCollectionElement element : elements )
		{
			if ( element.open( params.virtual ) == null )
				return null;
		}
		
		// all ImagePlusTimePoints, each of them needs its own model
		ArrayList< ImagePlusTimePoint > listImp = new ArrayList< ImagePlusTimePoint >();
		for ( ImageCollectionElement element : elements )
			listImp.add( new ImagePlusTimePoint( element.open( params.virtual ), element.getIndex(), 1, element.getModel(), element ) );
	
		// get the connecting tiles
		Vector< ComparePair > overlappingTiles = new Vector< ComparePair >();
		
		// Added by John Lapage: if the sequential option has been chosen, pair up each image 
		// with the images within the specified range, and return.
		if ( params.sequential )
		{
			for ( int i = 0; i < elements.size(); i++ )
			{
				for ( int j = 1 ; j <= params.seqRange ; j++ )
				{
					if ( ( i + j ) >= elements.size() ) 
						break;
					
					overlappingTiles.add( new ComparePair( listImp.get( i ), listImp.get( i+j ) ) );
				}
			}
			return overlappingTiles;
		}
		// end of addition

		for ( int i = 0; i < elements.size() - 1; i++ )
			for ( int j = i + 1; j < elements.size(); j++ )
			{
				ImageCollectionElement e1 = elements.get( i );
				ImageCollectionElement e2 = elements.get( j );
				
				boolean overlapping = true;
				
				for ( int d = 0; d < params.dimensionality; ++d )
				{
					if ( !( ( e2.offset[ d ] >= e1.offset[ d ] && e2.offset[ d ] <= e1.offset[ d ] + e1.size[ d ] ) || 
						    ( e2.offset[ d ] + e2.size[ d ] >= e1.offset[ d ] && e2.offset[ d ] + e2.size[ d ] <= e1.offset[ d ] + e1.size[ d ] ) ||
						    ( e2.offset[ d ] <= e1.offset[ d ] && e2.offset[ d ] >= e1.offset[ d ] + e1.size[ d ] ) 
					   )  )
									overlapping = false;
				}
				
				if ( overlapping )
				{
					//ImagePlusTimePoint impA = new ImagePlusTimePoint( e1.open(), e1.getIndex(), 1, e1.getModel().copy(), e1 );
					//ImagePlusTimePoint impB = new ImagePlusTimePoint( e2.open(), e2.getIndex(), 1, e2.getModel().copy(), e2 );
					overlappingTiles.add( new ComparePair( listImp.get( i ), listImp.get( j ) ) );
				}
			}
		
		return overlappingTiles;
	}
}
