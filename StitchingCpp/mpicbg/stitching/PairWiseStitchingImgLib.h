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

import fiji.stacks.Hyperstack_rearranger;
import ij.IJ;
import ij.ImagePlus;
import ij.gui.Roi;

import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicInteger;

import stitching.utils.Log;
import mpicbg.imglib.algorithm.fft.PhaseCorrelation;
import mpicbg.imglib.algorithm.fft.PhaseCorrelationPeak;
import mpicbg.imglib.algorithm.scalespace.DifferenceOfGaussianPeak;
import mpicbg.imglib.algorithm.scalespace.SubpixelLocalization;
import mpicbg.imglib.cursor.LocalizableByDimCursor;
import mpicbg.imglib.cursor.LocalizableCursor;
import mpicbg.imglib.image.Image;
import mpicbg.imglib.image.ImageFactory;
import mpicbg.imglib.image.display.imagej.ImageJFunctions;
import mpicbg.imglib.multithreading.Chunk;
import mpicbg.imglib.multithreading.SimpleMultiThreading;
import mpicbg.imglib.type.numeric.RealType;
import mpicbg.imglib.type.numeric.integer.UnsignedByteType;
import mpicbg.imglib.type.numeric.integer.UnsignedShortType;
import mpicbg.imglib.type.numeric.real.FloatType;

/**
 * Pairwise Stitching of two ImagePlus using ImgLib1 and PhaseCorrelation.
 * It deals with aligning two slices (2d) or stacks (3d) having an arbitrary
 * amount of channels. If the ImagePlus contains several time-points it will 
 * only consider the first time-point as this requires global optimization of 
 * many independent 2d/3d &lt;-&gt; 2d/3d alignments.
 * 
 * @author Stephan Preibisch (stephan.preibisch@gmx.de)
 *
 */
class PairWiseStitchingImgLib 
{
	public static PairWiseStitchingResult stitchPairwise( ImagePlus imp1, ImagePlus imp2, Roi roi1, Roi roi2, int timepoint1, int timepoint2, StitchingParameters params )
	{
		PairWiseStitchingResult result = null;
		roi1 = getOnlyRectangularRoi( roi1 );
		roi2 = getOnlyRectangularRoi( roi2 );
		
		// can both images be wrapped into imglib without copying
		boolean canWrap = !StitchingParameters.alwaysCopy && canWrapIntoImgLib( imp1, roi1, params.channel1 ) && canWrapIntoImgLib( imp2, roi2, params.channel2 );
		
		
		//
		// the ugly but correct way into generic programming...
		//
		if ( canWrap )
		{
			if ( imp1.getType() == ImagePlus.GRAY32 )
			{
				Image<FloatType> image1 = getWrappedImageFloat( imp1, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getWrappedImageFloat( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getWrappedImageUnsignedShort( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getWrappedImageUnsignedByte( imp2, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );
				
				image1.close();
			}
			else if ( imp1.getType() == ImagePlus.GRAY16 )
			{
				Image<UnsignedShortType> image1 = getWrappedImageUnsignedShort( imp1, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getWrappedImageFloat( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getWrappedImageUnsignedShort( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getWrappedImageUnsignedByte( imp2, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );

				image1.close();
			} 
			else if ( imp1.getType() == ImagePlus.GRAY8 )
			{
				Image<UnsignedByteType> image1 = getWrappedImageUnsignedByte( imp1, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getWrappedImageFloat( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getWrappedImageUnsignedShort( imp2, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getWrappedImageUnsignedByte( imp2, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );
				
				image1.close();
			} 
			else
			{
				LOGERR( "Unknown image type: " + imp1.getType() );			
			}
		}
		else
		{
			ImageFactory<UnsignedByteType> imgFactoryByte = new ImageFactory<UnsignedByteType>( new UnsignedByteType(), StitchingParameters.phaseCorrelationFactory );
			ImageFactory<UnsignedShortType> imgFactoryShort = new ImageFactory<UnsignedShortType>( new UnsignedShortType(), StitchingParameters.phaseCorrelationFactory );
			ImageFactory<FloatType> imgFactoryFloat = new ImageFactory<FloatType>( new FloatType(), StitchingParameters.phaseCorrelationFactory );
			
			if ( imp1.getType() == ImagePlus.GRAY32 )
			{
				Image< FloatType > image1 = getImage( imp1, roi1, imgFactoryFloat, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryFloat, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryShort, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryByte, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );					
			}
			else if ( imp1.getType() == ImagePlus.GRAY16 )
			{
				Image< UnsignedShortType > image1 = getImage( imp1, roi1, imgFactoryShort, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryFloat, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryShort, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryByte, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );					
			}
			else if ( imp1.getType() == ImagePlus.GRAY8 )
			{
				Image< UnsignedByteType > image1 = getImage( imp1, roi1, imgFactoryByte, params.channel1, timepoint1 );
				
				if ( imp2.getType() == ImagePlus.GRAY32 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryFloat, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY16 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryShort, params.channel2, timepoint2 ), params );
				else if ( imp2.getType() == ImagePlus.GRAY8 )
					result = performStitching( image1, getImage( imp2, roi2, imgFactoryByte, params.channel2, timepoint2 ), params );
				else
					LOGERR( "Unknown image type: " + imp2.getType() );					
			}
			else
			{
				LOGERR( "Unknown image type: " + imp1.getType() );			
			}
		}
		if ( result == null )
		{
			LOGERR( "Pairwise stitching failed." );
			return null;
		}
		
		// add the offset to the shift
		if ( roi2 != null )
		{
			result.offset[ 0 ] -= roi2.getBounds().x;
			result.offset[ 1 ] -= roi2.getBounds().y;
		}	
		
		if ( roi1 != null )
		{
			result.offset[ 0 ] += roi1.getBounds().x;
			result.offset[ 1 ] += roi1.getBounds().y;			
		}
		
		return result;
	}

	public static < T : public RealType<T>, S : public RealType<S> > PairWiseStitchingResult performStitching( Image<T> img1, Image<S> img2, StitchingParameters params )
	{
		if ( img1 == null )
		{
			LOGERR( "Image 1 could not be wrapped." );
			return null;
		}
		else if ( img2 == null )
		{
			LOGERR( "Image 2 could not be wrapped." );
			return null;
		}
		else if ( params == null )
		{
			LOGERR( "Parameters are null." );
			return null;
		}
		
		PairWiseStitchingResult result = computePhaseCorrelation( img1, img2, params.checkPeaks, params.subpixelAccuracy );
		
		return result;
	}
	
	public static < T : public RealType<T>, S : public RealType<S> > PairWiseStitchingResult computePhaseCorrelation( Image<T> img1, Image<S> img2, int numPeaks, boolean subpixelAccuracy )
	{
		PhaseCorrelation< T, S > phaseCorr = new PhaseCorrelation<T, S>( img1, img2 );
		phaseCorr.setInvestigateNumPeaks( numPeaks );
		
		if ( subpixelAccuracy )
			phaseCorr.setKeepPhaseCorrelationMatrix( true );
		
		phaseCorr.setComputeFFTinParalell( true );
		if ( !phaseCorr.process() )
		{
			LOGERR( "Could not compute phase correlation: " + phaseCorr.getErrorMessage() );
			return null;
		}

		// result
		PhaseCorrelationPeak pcp = phaseCorr.getShift();
		float[] shift = new float[ img1.getNumDimensions() ];
		PairWiseStitchingResult result;
		
		if ( subpixelAccuracy )
		{
			Image<FloatType> pcm = phaseCorr.getPhaseCorrelationMatrix();		
		
			ArrayList<DifferenceOfGaussianPeak<FloatType>> list = new ArrayList<DifferenceOfGaussianPeak<FloatType>>();		
			Peak p = new Peak( pcp );
			list.add( p );
					
			SubpixelLocalization<FloatType> spl = new SubpixelLocalization<FloatType>( pcm, list );
			boolean move[] = new boolean[ pcm.getNumDimensions() ];
			for ( int i = 0; i < pcm.getNumDimensions(); ++i )
				move[ i ] = false;
			spl.setCanMoveOutside( true );
			spl.setAllowedToMoveInDim( move );
			spl.setMaxNumMoves( 0 );
			spl.setAllowMaximaTolerance( false );
			spl.process();
			
			Peak peak = (Peak)list.get( 0 );
			
			for ( int d = 0; d < img1.getNumDimensions(); ++d )
				shift[ d ] = peak.getPCPeak().getPosition()[ d ] + peak.getSubPixelPositionOffset( d );
			
			pcm.close();
			
			result = new PairWiseStitchingResult( shift, pcp.getCrossCorrelationPeak(), p.getValue().get() );
		}
		else
		{
			for ( int d = 0; d < img1.getNumDimensions(); ++d )
				shift[ d ] = pcp.getPosition()[ d ];
			
			result = new PairWiseStitchingResult( shift, pcp.getCrossCorrelationPeak(), pcp.getPhaseCorrelationPeak() );
		}
		
		return result;
	}

	/**
	 * return an {@code Image<T>} as input for the PhaseCorrelation.
	 * 
	 * @param imp - the {@link ImagePlus}
	 * @param imgFactory - the {@link ImageFactory} defining wher to put it into
	 * @param channel - which channel (if channel=0 means average all channels)
	 * @param timepoint - which timepoint
	 * 
	 * @return - the {@link Image} or null if it was not an ImagePlus.GRAY8, ImagePlus.GRAY16 or ImagePlus.GRAY32
	 */
	public static < T : public RealType<T> > Image<T> getImage( ImagePlus imp, Roi roi, ImageFactory<T> imgFactory, int channel, int timepoint )
	{
		// first test the roi
		roi = getOnlyRectangularRoi( roi );
		
		// how many dimensions?
		int numDimensions;		
		if ( imp.getNSlices() > 1 )
			numDimensions = 3;
		else
			numDimensions = 2;
		
		// the size of the image
		int[] size = new int[ numDimensions ];
		int[] offset = new int[ numDimensions ];
		
		if ( roi == null )
		{
			size[ 0 ] = imp.getWidth();
			size[ 1 ] = imp.getHeight();
			
			if ( numDimensions == 3 )
				size[ 2 ] = imp.getNSlices();
		}
		else
		{
			size[ 0 ] = roi.getBounds().width;
			size[ 1 ] = roi.getBounds().height;

			offset[ 0 ] = roi.getBounds().x;
			offset[ 1 ] = roi.getBounds().y;
			
			if ( numDimensions == 3 )
				size[ 2 ] = imp.getNSlices();
		}
		
		// create the Image
		Image<T> img = imgFactory.createImage( size );
		boolean success;
		
		// copy the content
		if ( channel == 0 )
		{
			// we need to average all channels
			success = averageAllChannels( img, offset, imp, timepoint );
		}
		else
		{
			// otherwise only copy one channel
			success = fillInChannel( img, offset, imp, channel, timepoint );
		}
		
		if ( success )
		{
			return img;
		}
		img.close();
		return null;
	}
	
	/**
	 * Averages all channels into the target image. The size is given by the dimensions of the target image,
	 * the offset (if applicable) is given by an extra field
	 * 
	 * @param target - the target Image
	 * @param offset - the offset of the area (might be [0,0] or [0,0,0])
	 * @param imp - the input ImagePlus
	 * @param timepoint - for which timepoint
	 * 
	 * @return true if successful, false if the ImagePlus type was unknow
	 */
	public static < T : public RealType< T > > boolean averageAllChannels( Image< T > target, int[] offset, ImagePlus imp, int timepoint )
	{
		int numChannels = imp.getNChannels();
		
		if ( imp.getType() == ImagePlus.GRAY8 )
		{
			ArrayList< Image< UnsignedByteType > > images = new ArrayList<Image<UnsignedByteType>>();

			// first get wrapped instances of all channels
			for ( int c = 1; c <= numChannels; ++c )
				images.add( getWrappedImageUnsignedByte( imp, c, timepoint ) );			

			averageAllChannels( target, images, offset );			
			return true;
		}
		else if ( imp.getType() == ImagePlus.GRAY16 )
		{
			ArrayList< Image< UnsignedShortType > > images = new ArrayList<Image<UnsignedShortType>>();

			// first get wrapped instances of all channels
			for ( int c = 1; c <= numChannels; ++c )
				images.add( getWrappedImageUnsignedShort( imp, c, timepoint ) );			

			averageAllChannels( target, images, offset );
			return true;
		}
		else if ( imp.getType() == ImagePlus.GRAY32 )
		{
			ArrayList< Image< FloatType > > images = new ArrayList<Image<FloatType>>();

			// first get wrapped instances of all channels
			for ( int c = 1; c <= numChannels; ++c )
				images.add( getWrappedImageFloat( imp, c, timepoint ) );
			
			averageAllChannels( target, images, offset );
			return true;
		}
		else
		{
			LOGERR( "Unknow image type: " + imp.getType() );
			return false;
		}
	}

	/**
	 * Averages all channels into the target image. The size is given by the dimensions of the target image,
	 * the offset (if applicable) is given by an extra field
	 * 
	 * @param target - the target Image
	 * @param offset - the offset of the area (might be [0,0] or [0,0,0])
	 * @param imp - the input ImagePlus
	 * @param timepoint - for which timepoint
	 * 
	 * @return true if successful, false if the ImagePlus type was unknow
	 */
	public static < T : public RealType< T > > boolean fillInChannel( Image< T > target, int[] offset, ImagePlus imp, int channel, int timepoint )
	{
		if ( imp.getType() == ImagePlus.GRAY8 )
		{
			ArrayList< Image< UnsignedByteType > > images = new ArrayList<Image<UnsignedByteType>>();

			// first get wrapped instances of all channels
			images.add( getWrappedImageUnsignedByte( imp, channel, timepoint ) );			

			averageAllChannels( target, images, offset );			
			return true;
		}
		else if ( imp.getType() == ImagePlus.GRAY16 )
		{
			ArrayList< Image< UnsignedShortType > > images = new ArrayList<Image<UnsignedShortType>>();

			// first get wrapped instances of all channels
			images.add( getWrappedImageUnsignedShort( imp, channel, timepoint ) );			

			averageAllChannels( target, images, offset );
			return true;
		}
		else if ( imp.getType() == ImagePlus.GRAY32 )
		{
			ArrayList< Image< FloatType > > images = new ArrayList<Image<FloatType>>();

			// first get wrapped instances of all channels
			images.add( getWrappedImageFloat( imp, channel, timepoint ) );
			
			averageAllChannels( target, images, offset );
			return true;
		}
		else
		{
			LOGERR( "Unknow image type: " + imp.getType() );
			return false;
		}
	}

	/**
	 * Averages all channels into the target image. The size is given by the dimensions of the target image,
	 * the offset (if applicable) is given by an extra field
	 * 
	 * @param target - the target Image
	 * @param offset - the offset of the area (might be [0,0] or [0,0,0])
	 * @param sources - a list of input Images
	 */
	protected static < T : public RealType< T >, S : public RealType< S > > void averageAllChannels( Image< T > target, ArrayList< Image< S > > sources, int[] offset )
	{
		// get the major numbers
		int numDimensions = target.getNumDimensions();
		float numImages = sources.size();
		long imageSize = target.getDimension( 0 );
		
		for ( int d = 1; d < target.getNumDimensions(); ++d )
			imageSize *= target.getDimension( d );

		// run multithreaded
		AtomicInteger ai = new AtomicInteger(0);					
        Thread[] threads = SimpleMultiThreading.newThreads();

        Vector<Chunk> threadChunks = SimpleMultiThreading.divideIntoChunks( imageSize, threads.length );
        
        for (int ithread = 0; ithread < threads.length; ++ithread)
            threads[ithread] = new Thread(new Runnable()
            {
                @Override
                public void run()
                {
                	// Thread ID
                	int myNumber = ai.getAndIncrement();
        
                	// get chunk of pixels to process
                	Chunk myChunk = threadChunks.get( myNumber );
                	long startPos = myChunk.getStartPosition();
                	long loopSize = myChunk.getLoopSize();
                	
            		// the cursor for the output
            		LocalizableCursor< T > targetCursor =  target.createLocalizableCursor();
            		
            		// the input cursors
            		ArrayList< LocalizableByDimCursor< S > > sourceCursors = new ArrayList< LocalizableByDimCursor< S > > ();
            		
            		for ( Image< S > source : sources )
            			sourceCursors.add( source.createLocalizableByDimCursor() );
            		
            		// temporary array
            		int[] location = new int[ numDimensions ]; 

            		// move to the starting position of the current thread
            		targetCursor.fwd( startPos );
                    
            		// do as many pixels as wanted by this thread
                    for ( long j = 0; j < loopSize; ++j )
            		{
            			targetCursor.fwd();
            			targetCursor.getPosition( location );
            			
            			for ( int d = 0; d < numDimensions; ++d )
            				location[ d ] += offset[ d ];
            			
            			float sum = 0;
            			
            			for ( LocalizableByDimCursor< S > sourceCursor : sourceCursors )
            			{
            				sourceCursor.setPosition( location );
            				sum += sourceCursor.getType().getRealFloat();
            			}
            			
            			targetCursor.getType().setReal( sum / numImages );
            		}                	
                }
            });
        
        SimpleMultiThreading.startAndJoin( threads );		
	}

	/**
	 * return an {@link Image} of {@link UnsignedByteType} as input for the PhaseCorrelation. If no rectangular roi
	 * is selected, it will only wrap the existing ImagePlus!
	 * 
	 * @param imp - the {@link ImagePlus}
	 * @param channel - which channel (if channel=0 means average all channels)
	 * @param timepoint - which timepoint
	 * 
	 * @return - the {@link Image} or null if it was not an ImagePlus.GRAY8 or if channel = 0
	 */
	public static Image<UnsignedByteType> getWrappedImageUnsignedByte( ImagePlus imp, int channel, int timepoint )
	{
		if ( channel == 0 || imp.getType() != ImagePlus.GRAY8 )
			return null;
		return ImageJFunctions.wrapByte( Hyperstack_rearranger.getImageChunk( imp, channel, timepoint ) );
	}

	/**
	 * return an {@link Image} of {@link UnsignedShortType} as input for the PhaseCorrelation. If no rectangular roi
	 * is selected, it will only wrap the existing ImagePlus!
	 * 
	 * @param imp - the {@link ImagePlus}
	 * @param channel - which channel (if channel=0 means average all channels)
	 * @param timepoint - which timepoint
	 * 
	 * @return - the {@link Image} or null if it was not an ImagePlus.GRAY16 or if channel = 0
	 */
	public static Image<UnsignedShortType> getWrappedImageUnsignedShort( ImagePlus imp, int channel, int timepoint )
	{
		if ( channel == 0 || imp.getType() != ImagePlus.GRAY16 )
			return null;
		return ImageJFunctions.wrapShort( Hyperstack_rearranger.getImageChunk( imp, channel, timepoint ) );
	}

	/**
	 * return an {@link Image} of {@link FloatType} as input for the PhaseCorrelation. If no rectangular roi
	 * is selected, it will only wrap the existing ImagePlus!
	 * 
	 * @param imp - the {@link ImagePlus}
	 * @param channel - which channel (if channel=0 means average all channels)
	 * @param timepoint - which timepoint
	 * 
	 * @return - the {@link Image} or null if it was not an ImagePlus.GRAY32 or if channel = 0
	 */
	public static Image<FloatType> getWrappedImageFloat( ImagePlus imp, int channel, int timepoint )
	{
		if ( channel == 0 || imp.getType() != ImagePlus.GRAY32 )
			return null;
		return ImageJFunctions.wrapFloat( Hyperstack_rearranger.getImageChunk( imp, channel, timepoint ) );
	}

	/**
	 * Determines if this imageplus with these parameters can be wrapped directly into an {@code Image<T>}.
	 * This is important, because if we would wrap the first but not the second image, they would
	 * have different {@link ImageFactory}s
	 * 
	 * @param imp - the ImagePlus
	 * @param roi
	 * @param channel - which channel (if channel=0 means average all channels)
	 * 
	 * @return true if it can be wrapped, otherwise false
	 */
	public static boolean canWrapIntoImgLib( ImagePlus imp, Roi roi, int channel )
	{
		// first test the roi
		roi = getOnlyRectangularRoi( roi );
		
		return roi == null && channel > 0;
	}

	protected static Roi getOnlyRectangularRoi( Roi roi )
	{
		// we can only do rectangular rois
		if ( roi != null && roi.getType() != Roi.RECTANGLE )
			return null;
		return roi;
	}

	/*
	protected static Roi getOnlyRectangularRoi( ImagePlus imp )
	{
		Roi roi = imp.getRoi();
		
		// we can only do rectangular rois
		if ( roi != null && roi.getType() != Roi.RECTANGLE )
		{
			Log.warn( "roi for " + imp.getTitle() + " is not a rectangle, we have to ignore it." );
			roi = null;
		}

		return roi;
	}
	*/
}
