#pragma once

#include "header.h"
#include "ContainerFactory.h"



class PixelGridContainerFactory : public ContainerFactory
{
public:
	PixelGridContainerFactory : public ContainerFactory();
	~PixelGridContainerFactory : public ContainerFactory();
	template<class T>
	virtual <T extends Type<T>> PixelGridContainer<T> createContainer(int dim[], T type) = 0;
private:

};

PixelGridContainerFactory : public ContainerFactory::PixelGridContainerFactory : public ContainerFactory()
{
}

PixelGridContainerFactory : public ContainerFactory::~PixelGridContainerFactory : public ContainerFactory()
{
}