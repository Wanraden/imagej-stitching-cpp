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
#include "PixelFusion.h"

class AveragePixelFusion : public PixelFusion
{
protected:
	double avg;
	int count;

public:
	AveragePixelFusion() { clear(); }

	virtual void clear() override
	{
		count = 0;
		avg = 0;
	}


	virtual void addValue(double value, int imageId, double localPosition[]) override
	{
		avg += value;
		++count;
	}


	virtual double getValue()
	{
		if (count == 0)
			return 0;
		return (avg / count);
	}


	virtual PixelFusion* copy() override { return new AveragePixelFusion(); }
};
