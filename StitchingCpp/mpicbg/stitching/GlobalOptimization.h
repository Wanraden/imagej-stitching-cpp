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

import java.util.ArrayList;
import java.util.Collections;
import java.util.Set;
import java.util.Vector;

import mpicbg.models.Point;
import mpicbg.models.PointMatch;
import mpicbg.models.Tile;
import mpicbg.models.TranslationModel2D;
import mpicbg.models.TranslationModel3D;
import stitching.utils.Log;


class GlobalOptimization 
{
	public static boolean ignoreZ = false;
	
	public static ArrayList< ImagePlusTimePoint > optimize( Vector< ComparePair > pairs, ImagePlusTimePoint fixedImage, StitchingParameters params )
	{
		boolean redo;
		TileConfigurationStitching tc;
		do
		{
			redo = false;
			ArrayList< Tile< ? > > tiles = new ArrayList< Tile< ? > >();
			
			for ( ComparePair pair : pairs )
			{
				if ( pair.getCrossCorrelation() >= params.regThreshold && pair.getIsValidOverlap() )
				{
					Tile t1 = pair.getTile1();
					Tile t2 = pair.getTile2();
					
					Point p1, p2;
					
					if ( params.dimensionality == 3 )
					{
						// the transformations that map each tile into the relative global coordinate system (that's why the "-")
						p1 = new Point( new double[]{ 0,0,0 } );
						
						if ( ignoreZ )
							p2 = new Point( new double[]{ -pair.getRelativeShift()[ 0 ], -pair.getRelativeShift()[ 1 ], 0 } );
						else
							p2 = new Point( new double[]{ -pair.getRelativeShift()[ 0 ], -pair.getRelativeShift()[ 1 ], -pair.getRelativeShift()[ 2 ] } );
					}
					else 
					{
						p1 = new Point( new double[]{ 0, 0 } );
						p2 = new Point( new double[]{ -pair.getRelativeShift()[ 0 ], -pair.getRelativeShift()[ 1 ] } );						
					}
					
					t1.addMatch( new PointMatchStitching( p1, p2, pair.getCrossCorrelation(), pair ) );
					t2.addMatch( new PointMatchStitching( p2, p1, pair.getCrossCorrelation(), pair ) );
					t1.addConnectedTile( t2 );
					t2.addConnectedTile( t1 );
					
					if (!tiles.contains(t1))
						tiles.add( t1 );
	
					if (!tiles.contains(t2))
						tiles.add( t2 );			
					
					pair.setIsValidOverlap( true );
				}
				else
				{
					pair.setIsValidOverlap( false );
				}
			}
			
			if ( tiles.size() == 0 )
			{
				
				if ( params.dimensionality == 3 )
				{
					LOGERR( "Error: No correlated tiles found, setting the first tile to (0, 0, 0)." );
					TranslationModel3D model = (TranslationModel3D)fixedImage.getModel();
					model.set( 0, 0, 0 );
				}
				else
				{
					LOGERR( "Error: No correlated tiles found, setting the first tile to (0, 0)." );
					TranslationModel2D model = (TranslationModel2D)fixedImage.getModel();
					model.set( 0, 0 );					
				}
				
				ArrayList< ImagePlusTimePoint > imageInformationList = new ArrayList< ImagePlusTimePoint >();
				imageInformationList.add( fixedImage );
				
				LOGINFO(" number of tiles = " + imageInformationList.size() );
				
				return imageInformationList;
			}						

			tc = new TileConfigurationStitching();
			tc.addTiles( tiles );
			
			// find a useful fixed tile
			if ( fixedImage.getConnectedTiles().size() > 0 )
			{
				tc.fixTile( fixedImage );
			}
			else
			{
				for ( int i = 0; i < tiles.size(); ++i )
					if ( tiles.get( i ).getConnectedTiles().size() > 0 )
					{
						tc.fixTile( tiles.get( i ) );
						break;
					}
			}

			try
			{
				tc.preAlign();
				tc.optimize( 10, 1000, 200 );

				double avgError = tc.getError();
				double maxError = tc.getMaxError();				

				if ( ( ( avgError*params.relativeThreshold < maxError && maxError > 0.95 ) || avgError > params.absoluteThreshold ) )
				{
					double longestDisplacement = 0;
					PointMatch worstMatch = null;

					// new way of finding biggest error to look for the largest displacement
					for ( Tile t : tc.getTiles() )
					{
						for ( PointMatch p :  (Set< PointMatch >)t.getMatches() )
						{
							if ( p.getDistance() > longestDisplacement )
							{
								longestDisplacement = p.getDistance();
								worstMatch = p;
							}
						}
					}

					ComparePair pair = ((PointMatchStitching)worstMatch).getPair();
					
					LOGINFO( "Identified link between " + pair.getImagePlus1().getTitle() + "[" + pair.getTile1().getTimePoint() + "] and " + 
							pair.getImagePlus2().getTitle() + "[" + pair.getTile2().getTimePoint() + "] (R=" + pair.getCrossCorrelation() +") to be bad. Reoptimizing.");
					
					((PointMatchStitching)worstMatch).getPair().setIsValidOverlap( false );
					redo = true;
					
					for ( Tile< ? > t : tiles )
					{
						t.getConnectedTiles().clear();
						t.getMatches().clear();
					}
				}
			}
			catch ( Exception e )
			{ 
				LOGERR( "Cannot compute global optimization: " + e, e );
			}
		}
		while(redo);
		
		
		// create a list of image informations containing their positions			
		ArrayList< ImagePlusTimePoint > imageInformationList = new ArrayList< ImagePlusTimePoint >();
		for ( Tile< ? > t : tc.getTiles() )
			imageInformationList.add( (ImagePlusTimePoint)t );
		
		Collections.sort( imageInformationList );
		
		return imageInformationList;
	}
}
