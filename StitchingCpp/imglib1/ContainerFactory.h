#pragma once
#include "header.h"
#include "Factory.h"

class ContainerFactory : public Factory {
protected:
	bool useOptimizedContainers = true;

public:
	/**
	 * This method is called by {@link Image}. The {@link ContainerFactory} can decide how to create the {@link Container},
	 * if it is for example a {@link DirectAccessContainerFactory} it will ask the {@link Type} to create a
	 * suitable {@link Container} for the {@link Type} and the dimensionality
	 *
	 * @return {@link Container} - the instantiated Container
	 */
	template<class T>
	virtual <T extends Type<T>> Container<T> createContainer(int dim[], T type) = 0;

	void setOptimizedContainerUse(bool useOptimizedContainers) { this->useOptimizedContainers = useOptimizedContainers; }
	bool useOptimizedContainers() { return useOptimizedContainers; }
};