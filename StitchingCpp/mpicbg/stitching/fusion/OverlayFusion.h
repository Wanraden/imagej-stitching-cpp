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

import fiji.stacks.Hyperstack_rearranger;
import ij.CompositeImage;
import ij.IJ;
import ij.ImagePlus;
import ij.ImageStack;
import ij.process.ImageProcessor;

import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicInteger;

import mpicbg.models.InvertibleBoundable;
import mpicbg.models.InvertibleCoordinateTransform;
import mpicbg.models.NoninvertibleModelException;
import net.imglib2.Cursor;
import net.imglib2.RandomAccessible;
import net.imglib2.RealRandomAccess;
import net.imglib2.RealRandomAccessible;
import net.imglib2.exception.ImgLibException;
import net.imglib2.img.Img;
import net.imglib2.img.ImgFactory;
import net.imglib2.img.display.imagej.ImageJFunctions;
import net.imglib2.img.imageplus.ImagePlusImg;
import net.imglib2.img.imageplus.ImagePlusImgFactory;
import net.imglib2.interpolation.InterpolatorFactory;
import net.imglib2.interpolation.randomaccess.NLinearInterpolatorFactory;
import net.imglib2.multithreading.Chunk;
import net.imglib2.multithreading.SimpleMultiThreading;
import net.imglib2.type.NativeType;
import net.imglib2.type.numeric.RealType;
import net.imglib2.type.numeric.real.FloatType;
import net.imglib2.view.Views;
import stitching.utils.CompositeImageFixer;
import stitching.utils.Log;

class OverlayFusion 
{
	protected static < T : public RealType< T > & NativeType< T > > CompositeImage createOverlay( T targetType, ImagePlus imp1, ImagePlus imp2, InvertibleBoundable finalModel1, InvertibleBoundable finalModel2, int dimensionality ) 
	{
		ArrayList< ImagePlus > images = new ArrayList<ImagePlus>();
		images.add( imp1 );
		images.add( imp2 );
		
		ArrayList< InvertibleBoundable > models = new ArrayList<InvertibleBoundable>();
		models.add( finalModel1 );
		models.add( finalModel2 );
		
		return createOverlay( targetType, images, models, dimensionality, 1, new NLinearInterpolatorFactory<FloatType>() );
	}
	
	public static < T : public RealType< T > & NativeType< T > > ImagePlus createReRegisteredSeries( T targetType, ImagePlus imp, ArrayList<InvertibleBoundable> models, int dimensionality )
	{
		int numImages = imp.getNFrames();

		// the size of the new image
		int[] size = new int[ dimensionality ];
		// the offset relative to the output image which starts with its local coordinates (0,0,0)
		double[] offset = new double[ dimensionality ];

		int[][] imgSizes = new int[ numImages ][ dimensionality ];
		
		for ( int i = 0; i < numImages; ++i )
		{
			imgSizes[ i ][ 0 ] = imp.getWidth();
			imgSizes[ i ][ 1 ] = imp.getHeight();
			if ( dimensionality == 3 )
				imgSizes[ i ][ 2 ] = imp.getNSlices();
		}
		
		// estimate the boundaries of the output image and the offset for fusion (negative coordinates after transform have to be shifted to 0,0,0)
		Fusion.estimateBounds( offset, size, imgSizes, models, dimensionality );
				
		// for output
		ImgFactory< T > f = new ImagePlusImgFactory< T >();
		// the composite
		ImageStack stack = new ImageStack( size[ 0 ], size[ 1 ] );

		for ( int t = 1; t <= numImages; ++t )
		{
			for ( int c = 1; c <= imp.getNChannels(); ++c )
			{
				Img<T> out = f.create( size, targetType );
				Img< FloatType > in = ImageJFunctions.convertFloat( Hyperstack_rearranger.getImageChunk( imp, c, t ) );

				fuseChannel( out, Views.interpolate( Views.extendZero( in ), new NLinearInterpolatorFactory< FloatType >() ), offset, models.get( t - 1 ) );

				try
				{
					ImagePlus outImp = ((ImagePlusImg<?,?>)out).getImagePlus();
					for ( int z = 1; z <= out.dimension( 2 ); ++z )
						stack.addSlice( imp.getTitle(), outImp.getStack().getProcessor( z ) );
				} 
				catch (ImgLibException e) 
				{
					LOGERR( "Output image has no ImageJ type: " + e );
				}				
			}
		}
		
		//convertXYZCT ...
		ImagePlus result = new ImagePlus( "registered " + imp.getTitle(), stack );
		
		// numchannels, z-slices, timepoints (but right now the order is still XYZCT)
		if ( dimensionality == 3 )
		{
			result.setDimensions( size[ 2 ], imp.getNChannels(), imp.getNFrames() );
			result = OverlayFusion.switchZCinXYCZT( result );
			return CompositeImageFixer.makeComposite( result, CompositeImage.COMPOSITE );
		}
		result.setDimensions( imp.getNChannels(), 1, imp.getNFrames() );
		
		if ( imp.getNChannels() > 1 )
			return CompositeImageFixer.makeComposite( result, CompositeImage.COMPOSITE );
		return result;
	}
	
	public static <T : public RealType<T> & NativeType<T>> CompositeImage createOverlay(
			T targetType,
			ArrayList<ImagePlus> images,
			ArrayList<InvertibleBoundable> models,
			int dimensionality,
			int timepoint,
			InterpolatorFactory< FloatType, RandomAccessible< FloatType > > factory )
	{	
		int numImages = images.size();
		
		// the size of the new image
		int[] size = new int[ dimensionality ];
		// the offset relative to the output image which starts with its local coordinates (0,0,0)
		double[] offset = new double[ dimensionality ];

		// estimate the boundaries of the output image and the offset for fusion (negative coordinates after transform have to be shifted to 0,0,0)
		Fusion.estimateBounds( offset, size, images, models, dimensionality );
		
		// for output
		ImgFactory<T> f = new ImagePlusImgFactory< T >();
		// the composite
		ImageStack stack = new ImageStack( size[ 0 ], size[ 1 ] );
		
		int numChannels = 0;
		
		//loop over all images
		for ( int i = 0; i < images.size(); ++i )
		{
			ImagePlus imp = images.get( i );
			
			// loop over all channels
			for ( int c = 1; c <= imp.getNChannels(); ++c )
			{
				Img<T> out = f.create( size, targetType );
				Img< FloatType > in = ImageJFunctions.convertFloat( Hyperstack_rearranger.getImageChunk( imp, c, timepoint ) );
				
				fuseChannel( out, Views.interpolate( Views.extendZero( in ), factory), offset, models.get( i + (timepoint - 1) * numImages ) );
				try 
				{
					ImagePlus outImp = ((ImagePlusImg<?,?>)out).getImagePlus();
					for ( int z = 1; z <= out.dimension( 2 ); ++z )
						stack.addSlice( imp.getTitle(), outImp.getStack().getProcessor( z ) );
				} 
				catch (ImgLibException e) 
				{
					LOGERR( "Output image has no ImageJ type: " + e );
				}
				
				// count all channels
				++numChannels;
			}
		}

		//convertXYZCT ...
		ImagePlus result = new ImagePlus( "overlay " + images.get( 0 ).getTitle() + " ... " + images.get( numImages - 1 ).getTitle(), stack );
		
		// numchannels, z-slices, timepoints (but right now the order is still XYZCT)
		if ( dimensionality == 3 )
		{
			result.setDimensions( size[ 2 ], numChannels, 1 );
			result = OverlayFusion.switchZCinXYCZT( result );
		}
		else
		{
			result.setDimensions( numChannels, 1, 1 );
		}
		
		return CompositeImageFixer.makeComposite( result, CompositeImage.COMPOSITE );
	}
		
	/**
	 * Fuse one slice/volume (one channel)
	 * 
	 * @param output - same the type of the ImagePlus input
	 * @param input - FloatType, because of Interpolation that needs to be done
	 * @param transform - the transformation
	 */
	protected static <T : public RealType<T>> void fuseChannel( Img<T> output, RealRandomAccessible<FloatType> input, double[] offset, InvertibleCoordinateTransform transform )
	{
		int dims = output.numDimensions();
		long imageSize = output.dimension( 0 );
		
		for ( int d = 1; d < output.numDimensions(); ++d )
			imageSize *= output.dimension( d );

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
                	
            		Cursor<T> out = output.localizingCursor();
            		RealRandomAccess<FloatType> in = input.realRandomAccess();
            		
            		double[] tmp = new double[ input.numDimensions() ];
            		
            		try 
            		{
                		// move to the starting position of the current thread
            			out.jumpFwd( startPos );
            			
                		// do as many pixels as wanted by this thread
                        for ( long j = 0; j < loopSize; ++j )
                        {
            				out.fwd();
            				
            				for ( int d = 0; d < dims; ++d )
            					tmp[ d ] = out.getDoublePosition( d ) + offset[ d ];
            				
            				transform.applyInverseInPlace( tmp );
            	
            				in.setPosition( tmp );
            				out.get().setReal( in.get().get() );
            			}
            		} 
            		catch (NoninvertibleModelException e) 
            		{
            			LOGERR( "Cannot invert model, qutting." );
            			return;
            		}

                }
            });
        
        SimpleMultiThreading.startAndJoin( threads );
		
        /*
		LocalizableCursor<T> out = output.createLocalizableCursor();
		Interpolator<FloatType> in = input.createInterpolator( factory );
		
		float[] tmp = new float[ input.getNumDimensions() ];
		
		try 
		{
			while ( out.hasNext() )
			{
				out.fwd();
				
				for ( int d = 0; d < dims; ++d )
					tmp[ d ] = out.getPosition( d ) + offset[ d ];
				
				transform.applyInverseInPlace( tmp );
	
				in.setPosition( tmp );			
				out.getType().setReal( in.getType().get() );
			}
		} 
		catch (NoninvertibleModelException e) 
		{
			LOGERR( "Cannot invert model, qutting." );
			return;
		}
		*/
	}

			
	/**
	 * Rearranges an ImageJ XYCZT Hyperstack into XYZCT without wasting memory for processing 3d images as a chunk,
	 * if it is already XYZCT it will shuffle it back to XYCZT
	 * 
	 * @param imp - the input {@link ImagePlus}
	 * @return - an {@link ImagePlus} which can be the same instance if the image is XYZT, XYZ, XYT or XY - otherwise a new instance
	 * containing the same processors but in the new order XYZCT
	 */
	public static ImagePlus switchZCinXYCZT( ImagePlus imp )
	{
		int numChannels = imp.getNChannels();
		int numTimepoints = imp.getNFrames();
		int numZStacks = imp.getNSlices();

		String newTitle;
		if ( imp.getTitle().startsWith( "[XYZCT]" ) )
			newTitle = imp.getTitle().substring( 8, imp.getTitle().length() );
		else
			newTitle = "[XYZCT] " + imp.getTitle();

		// there is only one channel and one z-stack, i.e. XY(T)
		if ( numChannels == 1 && numZStacks == 1 )
		{
			return imp;
		}
		// there is only one channel XYZ(T) or one z-stack XYC(T)
		else if ( numChannels == 1 || numZStacks == 1 )
		{
			// numchannels, z-slices, timepoints 
			// but of course now reversed...
			imp.setDimensions( numZStacks, numChannels, numTimepoints );
			imp.setTitle( newTitle );
			return imp;
		}
		
		// now we have to rearrange
		ImageStack stack = new ImageStack( imp.getWidth(), imp.getHeight() );
		
		for ( int t = 1; t <= numTimepoints; ++t )
		{
			for ( int c = 1; c <= numChannels; ++c )
			{
				for ( int z = 1; z <= numZStacks; ++z )
				{
					int index = imp.getStackIndex( c, z, t );
					ImageProcessor ip = imp.getStack().getProcessor( index );
					stack.addSlice( imp.getStack().getSliceLabel( index ), ip );
				}
			}
		}
				
		ImagePlus result = new ImagePlus( newTitle, stack );
		// numchannels, z-slices, timepoints 
		// but of course now reversed...
		result.setDimensions( numZStacks, numChannels, numTimepoints );
		result.getCalibration().pixelWidth = imp.getCalibration().pixelWidth;
		result.getCalibration().pixelHeight = imp.getCalibration().pixelHeight;
		result.getCalibration().pixelDepth = imp.getCalibration().pixelDepth;
		CompositeImage composite = CompositeImageFixer.makeComposite( result );
		
		return composite;
	}

	/**
	 * Rearranges an ImageJ XYCTZ Hyperstack into XYCZT without wasting memory for processing 3d images as a chunk,
	 * if it is already XYCTZ it will shuffle it back to XYCZT
	 * 
	 * @param imp - the input {@link ImagePlus}
	 * @return - an {@link ImagePlus} which can be the same instance if the image is XYC, XYZ, XYT or XY - otherwise a new instance
	 * containing the same processors but in the new order XYZCT
	 */
	public static ImagePlus switchZTinXYCZT( ImagePlus imp )
	{
		int numChannels = imp.getNChannels();
		int numTimepoints = imp.getNFrames();
		int numZStacks = imp.getNSlices();
		
		String newTitle;
		if ( imp.getTitle().startsWith( "[XYCTZ]" ) )
			newTitle = imp.getTitle().substring( 8, imp.getTitle().length() );
		else
			newTitle = "[XYCTZ] " + imp.getTitle();

		// there is only one timepoint and one z-stack, i.e. XY(C)
		if ( numTimepoints == 1 && numZStacks == 1 )
		{
			return imp;
		}
		// there is only one channel XYZ(T) or one z-stack XYC(T)
		else if ( numTimepoints == 1 || numZStacks == 1 )
		{
			// numchannels, z-slices, timepoints 
			// but of course now reversed...
			imp.setDimensions( numChannels, numTimepoints, numZStacks );
			imp.setTitle( newTitle );
			return imp;
		}
		
		// now we have to rearrange
		ImageStack stack = new ImageStack( imp.getWidth(), imp.getHeight() );
		
		for ( int z = 1; z <= numZStacks; ++z )
		{
			for ( int t = 1; t <= numTimepoints; ++t )
			{
				for ( int c = 1; c <= numChannels; ++c )
				{
					int index = imp.getStackIndex( c, z, t );
					ImageProcessor ip = imp.getStack().getProcessor( index );
					stack.addSlice( imp.getStack().getSliceLabel( index ), ip );
				}
			}
		}
		
		ImagePlus result = new ImagePlus( newTitle, stack );
		// numchannels, z-slices, timepoints 
		// but of course now reversed...
		result.setDimensions( numChannels, numTimepoints, numZStacks );
		result.getCalibration().pixelWidth = imp.getCalibration().pixelWidth;
		result.getCalibration().pixelHeight = imp.getCalibration().pixelHeight;
		result.getCalibration().pixelDepth = imp.getCalibration().pixelDepth;
		CompositeImage composite = CompositeImageFixer.makeComposite(  result );
		
		return composite;
	}

}
