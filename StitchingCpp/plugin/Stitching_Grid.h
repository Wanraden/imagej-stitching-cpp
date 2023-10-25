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
#include "header.h"
#include "tools/timehelper.h"
#include "mpicbg/stitching/StitchingParameters.h"
#include "mpicbg/stitching/ImageCollectionElement.h"
#include "mpicbg/stitching/ImagePlusTimePoint.h"

//import ij.ImagePlus;
//import ij.io.FileSaver;
//import java.io.File;
//import java.util.vector;
//import mpicbg.models.InvertibleBoundable;
//import mpicbg.models.TranslationModel2D;
//import mpicbg.models.TranslationModel3D;
//import mpicbg.stitching.*;
//import mpicbg.stitching.fusion.Fusion;
//import net.imglib2.type.numeric.integer.UnsignedByteType;
//import net.imglib2.type.numeric.integer.UnsignedShortType;
//import net.imglib2.type.numeric.real.FloatType;

/**
 *
 * @author Stephan Preibisch (stephan.preibisch@gmx.de)
 *
 */
class Stitching_Grid
{
public:
	int defaultFusionMethod = 0;
	double defaultRegressionThreshold = 0.3;
	double defaultDisplacementThresholdRelative = 2.5;
	double defaultDisplacementThresholdAbsolute = 3.5;
	int defaultMemorySpeedChoice = 1;

public:
	void run(vector<string> args, vector<string> files)
	{
		string resultDir = args[0];  // save result directory
		string resultFile = args[1]; // save result name
		int gridType = atoi(args[2].data());
		int gridOrder = atoi(args[3].data());
		int gridSizeX = atoi(args[4].data());
		int gridSizeY = atoi(args[5].data());
		double invalidScale = atof(args[6].data()); // image height scale
		double overlapX = atof(args[7].data());
		double overlapY = atof(args[7].data());

		if (gridType == -1 || gridOrder == -1)
			return;

		// the general stitching parameters
		StitchingParameters params;

		if (gridType == 4)
		{
			gridSizeX = 0;
			gridSizeY = 0;
			overlapX = 0;
			overlapY = 0;
		}

		params.fusionMethod = defaultFusionMethod;
		params.regThreshold = defaultRegressionThreshold;
		params.relativeThreshold = defaultDisplacementThresholdRelative;
		params.absoluteThreshold = defaultDisplacementThresholdAbsolute;
		params.computeOverlap = true;
		params.cpuMemChoice = defaultMemorySpeedChoice;
		// bool invertX = params.invertX;
		// bool invertY = params.invertY;
		// bool ignoreZStage = params.ignoreZStage;

		long startTime = TimeHelper::milliseconds();

		// we need to set this
		params.channel1 = 0;
		params.channel2 = 0;
		params.timeSelect = 0;
		params.checkPeaks = 5;

		// get all imagecollectionelements
		vector< ImageCollectionElement > elements;
		if (gridType < 4)
			elements = getGridLayout(gridType, gridOrder, gridSizeX, gridSizeY, overlapX, overlapY, files);
		//John Lapage modified this: copying setup for Unknown Positions
		else if (gridType == 4)
			elements = getAllFilesInArgs(files);

		if (elements.empty())
		{
			LOGERR("Error during tile discovery, or invalid grid type. Aborting.");
			return;
		}
		if (elements.size() < 2)
		{
			LOGERR("Found: " << elements.size() << " tiles, but at least 2 are required for stitching. Aborting.");
			return;
		}

		// open all images (if not done already by grid parsing) and test them, collect information
		int numChannels = -1;
		int numTimePoints = -1;

		bool is2d = false;
		bool is3d = false;

		for (ImageCollectionElement& element : elements)
		{
			LOGINFO("Loading: " << element.getFile().getAbsolutePath() << " ... ");

			long time = TimeHelper::milliseconds();
			ImagePlus imp = element.open(params.bVirtual);
			if (imp == nullptr)
				return;

			element.setSize({imp.getWidth(), (int)(imp.getHeight() * invalidScale)});

			time = TimeHelper::milliseconds() - time;

			int lastNumChannels = numChannels;
			int lastNumTimePoints = numTimePoints;
			numChannels = imp.getNChannels();
			numTimePoints = imp.getNFrames();

			if (imp.getNSlices() > 1)
			{
				LOGINFO(imp.getWidth() << "x" << imp.getHeight() << "x" << imp.getNSlices() << "px, channels=" << numChannels << ", timepoints=" << numTimePoints << " (" << time << " ms)");
				is3d = true;
			}
			else
			{
				LOGINFO(imp.getWidth() << "x" << imp.getHeight() << "px, channels=" << numChannels << ", timepoints=" << numTimePoints << " (" << time << " ms)");
				is2d = true;
			}

			// test validity of images
			if (is2d && is3d)
			{
				LOGERR("Some images are 2d, some are 3d ... cannot proceed");
				return;
			}

			if ((lastNumChannels != numChannels) && lastNumChannels != -1)
			{
				LOGERR("Number of channels per image changes ... cannot proceed");
				return;
			}

			if ((lastNumTimePoints != numTimePoints) && lastNumTimePoints != -1)
			{
				LOGERR("Number of timepoints per image changes ... cannot proceed");
				return;
			}

			// John Lapage changed this: copying setup for Unknown Positions
			if (gridType == 4)
			{
				if (is2d)
				{
					element.setDimensionality(2);
					element.setModel(new TranslationModel2D());
					element.setOffset({ 0, 0 });
				}
				else
				{
					element.setDimensionality(3);
					element.setModel(new TranslationModel3D());
					element.setOffset({ 0, 0, 0 });
				}
			}
		}

		// the dimensionality of each image that will be correlated (might still have more channels or timepoints)
		int dimensionality;

		if (is2d)
			dimensionality = 2;
		else
			dimensionality = 3;

		params.dimensionality = dimensionality;

		// call the stitching
		vector<ImagePlusTimePoint> optimized = CollectionStitchingImgLib.stitchCollection(elements, params);

		if (optimized.empty())
			return;

		// output the result
		for (ImagePlusTimePoint imt : optimized)
			LOGINFO(imt.getImagePlus().getTitle() << ": " << imt.getModel());

		// fuse
		{
			long time = TimeHelper::milliseconds();

			LOGINFO("Fuse & Display ...");

			// first prepare the models and get the target type
			vector<InvertibleBoundable> models = new vector< InvertibleBoundable >();
			vector<ImagePlus> images = new vector<ImagePlus>();

			bool is32bit = false;
			bool is16bit = false;
			bool is8bit = false;

			for (ImagePlusTimePoint imt : optimized)
			{
				ImagePlus imp = imt.getImagePlus();

				if (imp.getType() == ImagePlus.GRAY32)
					is32bit = true;
				else if (imp.getType() == ImagePlus.GRAY16)
					is16bit = true;
				else if (imp.getType() == ImagePlus.GRAY8)
					is8bit = true;

				images.add(imp);
			}

			for (int f = 1; f <= numTimePoints; ++f)
				for (ImagePlusTimePoint& imt : optimized)
					models.add((InvertibleBoundable)imt.getModel());

			ImagePlus *imp = nullptr;

			// test if there is no overlap between any of the tiles
			// if so fusion can be much faster
			bool noOverlap = false;

			if (is32bit)
				imp = Fusion.fuse(FloatType(), images, models, params.dimensionality, params.subpixelAccuracy, params.fusionMethod, params.outputDirectory, noOverlap, false, params.displayFusion);
			else if (is16bit)
				imp = Fusion.fuse(UnsignedShortType(), images, models, params.dimensionality, params.subpixelAccuracy, params.fusionMethod, params.outputDirectory, noOverlap, false, params.displayFusion);
			else if (is8bit)
				imp = Fusion.fuse(UnsignedByteType(), images, models, params.dimensionality, params.subpixelAccuracy, params.fusionMethod, params.outputDirectory, noOverlap, false, params.displayFusion);
			else
				LOGERR("Unknown image type for fusion.");

			LOGINFO("Finished fusion (" << (TimeHelper::milliseconds() - time) << " ms)");
			LOGINFO("Finished ... (" << (TimeHelper::milliseconds() - startTime) << " ms)");

			if (imp != nullptr)
				SavaFile(imp, resultDir, resultFile);
			else
				LOGINFO("images stitching failed");
		}

		// close all images
		for (ImageCollectionElement element : elements)
			element.close();
	}

	void SavaFile(ImagePlus imp, string path, string name)
	{
		FileSaver fs = new FileSaver(imp);
		LOGINFO(path << " " << name);
		File file = new File(path, name);
		string filename = file.getAbsolutePath();
		int lastDotIndex = filename.lastIndexOf(".");
		string extension = filename.substring(lastDotIndex + 1);
		switch (extension)
		{
		case "png":
			fs.saveAsPng(filename);
			break;
		case "bmp":
			fs.saveAsBmp(filename);
			break;
		case "jpg":
			fs.saveAsJpeg(filename);
			break;
		default:
			fs.saveAsTiff(filename);
			break;
		}
	}
	vector< ImageCollectionElement > getAllFilesInArgs(vector<string> imageFiles)
	{
		if (imageFiles.empty())
			return {};
		vector<string> files;
		for (string fileName : imageFiles)
		{
			File file = new File(fileName);

			if (file.isFile() && !file.isHidden() && !fileName.endsWith(".txt") && !fileName.endsWith(".TXT"))
			{
				files.push_back(fileName);
			}
		}

		LOGINFO("Found " << files.size() << " files (we ignore hidden and .txt files).");

		if (files.size() < 2)
		{
			LOGERR("Only " << files.size() << " files, you need at least 2 - stop.");
			return {};
		}

		vector< ImageCollectionElement > elements ;

		for (int i = 0; i < files.size(); ++i)
				elements.push_back(ImageCollectionElement(File(files.get(i)), i));

		if (elements.size() < 2)
		{
			LOGERR("Only " << elements.size() << " files selected, you need at least 2 - stop.");
			return {};
		}

		return elements;
	}

protected:
	vector< ImageCollectionElement > getGridLayout(int gridType, int gridOrder, int gridSizeX, int gridSizeY, double overlapX,
		double overlapY, vector<string> files)
	{
		// define the parsing of filenames
		// find how to parse
		// file position begin
		// determine the layout
		vector<vector<ImageCollectionElement>> gridLayout(gridSizeX, vector<ImageCollectionElement>(gridSizeY));// = new ImageCollectionElement[gridSizeX][gridSizeY];
		// all snakes, row, columns, whatever
		// the current position[x, y]
		int *position = new int[2];

		// we have gridSizeX * gridSizeY tiles
		for (int i = 0; i < gridSizeX * gridSizeY; ++i)
		{
			if (i > files.size())
				break;
			// get the vector where to move
			getPosition(position, i, gridType, gridOrder, gridSizeX, gridSizeY);
			// get the filename
			gridLayout[position[0]][position[1]] = ImageCollectionElement(new File(files[i]), i);
		}
		// file position end

		// based on the minimum size we will compute the initial arrangement
		int minWidth = INT_MAX;
		int minHeight = INT_MAX;
		int minDepth = INT_MAX;

		bool is2d = false;
		bool is3d = false;

		// open all images and test them, collect information
		for (int y = 0; y < gridSizeY; ++y)
			for (int x = 0; x < gridSizeX; ++x)
			{
				LOGINFO("Loading (" << x << ", " << y << "): " << gridLayout[x][y].getFile().getAbsolutePath() << " ... ");

				long time = TimeHelper::milliseconds();
				ImagePlus *imp = gridLayout[x][y].open(false);

				time = TimeHelper::milliseconds() - time;

				if (imp == nullptr)
					return {};

				if (imp.getNSlices() > 1)
				{
					LOGINFO(imp.getWidth() << "x" << imp.getHeight() << "x" << imp.getNSlices() << "px, channels=" << imp.getNChannels() << ", timepoints=" << imp.getNFrames() << " (" << time << " ms)");
					is3d = true;
				}
				else
				{
					LOGINFO(imp.getWidth() << "x" << imp.getHeight() << "px, channels=" << imp.getNChannels() << ", timepoints=" << imp.getNFrames() << " (" << time << " ms)");
					is2d = true;
				}

				// test validity of images
				if (is2d && is3d)
				{
					LOGINFO("Some images are 2d, some are 3d ... cannot proceed");
					return {};
				}

				if (imp.getWidth() < minWidth)
					minWidth = imp.getWidth();

				if (imp.getHeight() < minHeight)
					minHeight = imp.getHeight();

				if (imp.getNSlices() < minDepth)
					minDepth = imp.getNSlices();
			}

		int dimensionality;

		if (is3d)
			dimensionality = 3;
		else
			dimensionality = 2;

		// now get the approximate coordinates for each element
		// that is easiest done incrementally
		int xoffset = 0, yoffset = 0, zoffset = 0;

		// an vector containing all the ImageCollectionElements
		vector< ImageCollectionElement > elements;

		for (int y = 0; y < gridSizeY; y++)
		{
			if (y == 0)
				yoffset = 0;
			else
				yoffset += (int)(minHeight * (1 - overlapY));

			for (int x = 0; x < gridSizeX; x++)
			{
				ImageCollectionElement element = gridLayout[x][y];

				if (x == 0 && y == 0)
					xoffset = yoffset = zoffset = 0;

				if (x == 0)
					xoffset = 0;
				else
					xoffset += (int)(minWidth * (1 - overlapX));

				element.setDimensionality(dimensionality);

				if (dimensionality == 3)
				{
					element.setModel(TranslationModel3D());
					element.setOffset({ (float)xoffset, (float)yoffset, (float)zoffset });
				}
				else
				{
					element.setModel(TranslationModel2D());
					element.setOffset({ (float)xoffset, (float)yoffset });
				}

				elements.push_back(element);
			}
		}

		return elements;
	}

	// current snake directions ( if necessary )
	// they need a global state
	int snakeDirectionX = 0;
	int snakeDirectionY = 0;

	void getPosition(int currentPosition[], int i, int gridType, int gridOrder, int sizeX, int sizeY)
	{
		// gridType: "Row-by-row", "Column-by-column", "Snake by rows", "Snake by columns", "Fixed position"
		// gridOrder:
		//		choose2[ 0 ] = new vector<string>{ "Right & Down", "Left & Down", "Right & Up", "Left & Up" };
		//		choose2[ 1 ] = new vector<string>{ "Down & Right", "Down & Left", "Up & Right", "Up & Left" };
		//		choose2[ 2 ] = new vector<string>{ "Right & Down", "Left & Down", "Right & Up", "Left & Up" };
		//		choose2[ 3 ] = new vector<string>{ "Down & Right", "Down & Left", "Up & Right", "Up & Left" };

		// init the position
		if (i == 0)
		{
			if (gridOrder == 0 || gridOrder == 2)
				currentPosition[0] = 0;
			else
				currentPosition[0] = sizeX - 1;

			if (gridOrder == 0 || gridOrder == 1)
				currentPosition[1] = 0;
			else
				currentPosition[1] = sizeY - 1;

			// it is a snake
			if (gridType == 2 || gridType == 3)
			{
				// starting with right
				if (gridOrder == 0 || gridOrder == 2)
					snakeDirectionX = 1;
				else // starting with left
					snakeDirectionX = -1;

				// starting with down
				if (gridOrder == 0 || gridOrder == 1)
					snakeDirectionY = 1;
				else // starting with up
					snakeDirectionY = -1;
			}
		}
		else // a move is required
		{
			// row-by-row, "Right & Down", "Left & Down", "Right & Up", "Left & Up"
			if (gridType == 0)
			{
				// 0="Right & Down", 2="Right & Up"
				if (gridOrder == 0 || gridOrder == 2)
				{
					if (currentPosition[0] < sizeX - 1)
					{
						// just move one more right
						++currentPosition[0];
					}
					else
					{
						// we have to change rows
						if (gridOrder == 0)
							++currentPosition[1];
						else
							--currentPosition[1];

						// row-by-row going right, so only set position to 0
						currentPosition[0] = 0;
					}
				}
				else // 1="Left & Down", 3="Left & Up"
				{
					if (currentPosition[0] > 0)
					{
						// just move one more left
						--currentPosition[0];
					}
					else
					{
						// we have to change rows
						if (gridOrder == 1)
							++currentPosition[1];
						else
							--currentPosition[1];

						// row-by-row going left, so only set position to 0
						currentPosition[0] = sizeX - 1;
					}
				}
			}
			else if (gridType == 1) // col-by-col, "Down & Right", "Down & Left", "Up & Right", "Up & Left"
			{
				// 0="Down & Right", 1="Down & Left"
				if (gridOrder == 0 || gridOrder == 1)
				{
					if (currentPosition[1] < sizeY - 1)
					{
						// just move one down
						++currentPosition[1];
					}
					else
					{
						// we have to change columns
						if (gridOrder == 0)
							++currentPosition[0];
						else
							--currentPosition[0];

						// column-by-column going down, so position = 0
						currentPosition[1] = 0;
					}
				}
				else // 2="Up & Right", 3="Up & Left"
				{
					if (currentPosition[1] > 0)
					{
						// just move one up
						--currentPosition[1];
					}
					else
					{
						// we have to change columns
						if (gridOrder == 2)
							++currentPosition[0];
						else
							--currentPosition[0];

						// column-by-column going up, so position = sizeY - 1
						currentPosition[1] = sizeY - 1;
					}
				}
			}
			else if (gridType == 2) // "Snake by rows"
			{
				// currently going right
				if (snakeDirectionX > 0)
				{
					if (currentPosition[0] < sizeX - 1)
					{
						// just move one more right
						++currentPosition[0];
					}
					else
					{
						// just we have to change rows
						currentPosition[1] += snakeDirectionY;

						// and change the direction of the snake in x
						snakeDirectionX *= -1;
					}
				}
				else
				{
					// currently going left
					if (currentPosition[0] > 0)
					{
						// just move one more left
						--currentPosition[0];
						return;
					}
					// just we have to change rows
					currentPosition[1] += snakeDirectionY;

					// and change the direction of the snake in x
					snakeDirectionX *= -1;
				}
			}
			else if (gridType == 3) // "Snake by columns"
			{
				// currently going down
				if (snakeDirectionY > 0)
				{
					if (currentPosition[1] < sizeY - 1)
					{
						// just move one more down
						++currentPosition[1];
					}
					else
					{
						// we have to change columns
						currentPosition[0] += snakeDirectionX;

						// and change the direction of the snake in y
						snakeDirectionY *= -1;
					}
				}
				else
				{
					// currently going up
					if (currentPosition[1] > 0)
					{
						// just move one more up
						--currentPosition[1];
					}
					else
					{
						// we have to change columns
						currentPosition[0] += snakeDirectionX;

						// and change the direction of the snake in y
						snakeDirectionY *= -1;
					}
				}
			}
		}
	}
};

