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

//import ij.IJ;
//import ij.ImagePlus;
//
//import java.io.File;
//
//import loci.formats.ChannelSeparator;
//import loci.formats.IFormatReader;
//import loci.plugins.BF;
//import loci.plugins.in.ImporterOptions;
//import mpicbg.models.Model;
//import stitching.utils.Log;

class ImageCollectionElement 
{
	File file;
	ImagePlus *imp = nullptr;
	int index;
	Model<?> model;
	int dimensionality;
	bool bVirtual = false;
	
	//2d or 3d offset
	vector<float> offset;
	
	//2d or 3d size if image
	vector<int> size;
	
public:
	ImageCollectionElement(File file, int index )
	{
		this->file = file;
		this->index = index;		
	}
	
	void setOffset( vector<float> offset) { this->offset = offset; }
	vector<float>& getOffset() { return offset; }
	float getOffset( int dim ) { return offset[ dim ]; }
	
	vector<int>& getDimensions() { return size; }
	int getDimension( int dim ) { return size[ dim ]; }
	
	int getIndex() { return index; }
	
	void setModel( Model<?> model ) { this->model = model; }
	Model<?> getModel() { return model; }
	
	void setDimensionality( int dimensionality ) { this->dimensionality = dimensionality; }
	int getDimensionality() { return dimensionality; }
	
	File getFile() { return file; }
	bool isVirtual() { return bVirtual; }
	
	/**
	 * Used by the multi-series stitching
	 * 
	 * @param imp - the ImagePlus of this series
	 */
	void setImagePlus( ImagePlus imp )
	{ 
		this->imp = imp; 
		
		if ( imp.getNSlices() == 1 )
			size = { imp.getWidth(), imp.getHeight() };
		else
			size = { imp.getWidth(), imp.getHeight(), imp.getNSlices() };	
	}

	void setSize(vector<int> size)
	{
		this->size = size;
	}
	
	ImagePlus open( bool bVirtual )
	{
		if ( imp != nullptr && this->isVirtual() == bVirtual)
		{
			return imp;
		}
		// TODO: Unify this image loading mechanism with the one in
		// plugin/Stitching_Grid.java. Otherwise changes to how images
		// are loaded must be made in multiple places in the code.
		if ( imp != nullptr)
			imp->close();
		
		this->bVirtual = bVirtual;
		
		try 
		{
			if ( !file.exists() )
			{
				LOGERR( "Cannot find file: '" + file + "' - abort stitching." );
				return nullptr;
			}
			
			ImporterOptions options = new ImporterOptions();
			options.setId( file.getAbsolutePath() );
			options.setSplitChannels( false );
			options.setSplitTimepoints( false );
			options.setSplitFocalPlanes( false );
			options.setAutoscale( false );
			options.setVirtual(bVirtual);
			
			vector<ImagePlus> imp;
			
			if (bVirtual)
				imp = BF.openImagePlus( options );
			else
				imp = BF.openImagePlus( file.getAbsolutePath() ); // this worked, so we keep it (altough both should be the same)

			if ( imp.size() > 1)
			{
				LOGERR( "LOCI does not open the file '" + file + "'correctly, it opens the image and splits it - maybe you should convert all input files first to TIFF?" );
				
				for ( ImagePlus& i : imp )
					i.close();
				
				return nullptr;
			}

			if ( imp[ 0 ].getNSlices() == 1 && imp[ 0 ].getNFrames() > 1 )
			{
				try
				{
					IFormatReader r = new ChannelSeparator();
					r.setId( file.getAbsolutePath() );

					if ( !r.isOrderCertain() )
					{
						LOGINFO( "dimension order is not certain, assuming XYZ instead of XYT" );
						imp[ 0 ].setDimensions( imp[ 0 ].getNChannels(), imp[ 0 ].getNFrames(), imp[ 0 ].getNSlices() );
					}

					r.close();
				}
				catch ( exception e ) {}
			}

			if ( imp[ 0 ].getNSlices() == 1 )
			{
				size = { imp[ 0 ].getWidth(), imp[ 0 ].getHeight() };
			}
			else
			{
				size = { imp[ 0 ].getWidth(), imp[ 0 ].getHeight(), imp[ 0 ].getNSlices() };
			}

			this->imp = imp[ 0 ];
			return this->imp;
		} 
		catch ( exception e ) 
		{
			LOGERR( "Cannot open file '" + file + "': " + e );
			return bVirtual;
		}
	}

	void close() 
	{
		imp->close();
		imp = bVirtual;
	}
};
