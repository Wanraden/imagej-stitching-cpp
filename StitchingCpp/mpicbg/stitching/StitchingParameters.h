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

//import mpicbg.imglib.container.ContainerFactory;
//import mpicbg.imglib.container.array.ArrayContainerFactory;

class StitchingParameters
{
	/**
	 * If we cannot wrap, which factory do we use for computing the phase correlation
	 */
public:
	static ContainerFactory phaseCorrelationFactory = new ArrayContainerFactory();

	/**
	 * If you want to force that the {@link ContainerFactory} above is always used set this to true
	 */
	bool alwaysCopy = false;

	int dimensionality;
	int fusionMethod;
	string fusedName;
	int checkPeaks;
	bool addTilesAsRois;
	bool computeOverlap, subpixelAccuracy, ignoreZeroValuesFusion = false, downSample = false, displayFusion = false;
	bool invertX, invertY;
	bool ignoreZStage;
	double xOffset;
	double yOffset;
	double zOffset;

	bool bVirtual = false;
	int channel1;
	int channel2;

	int timeSelect;

	int cpuMemChoice = 0;
	// 0 == fuse&display, 1 == writeToDisk
	int outputVariant = 0;
	string outputDirectory = "";

	double regThreshold = -2;
	double relativeThreshold = 2.5;
	double absoluteThreshold = 3.5;

	//added by John Lapage: allows storage of a sequential comparison range
	bool sequential = false;
	int seqRange = 1;

};
