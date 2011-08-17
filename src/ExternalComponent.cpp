/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "AudioManager.h"
#include "ExternalComponent.h"

namespace xal
{
	void ExternalComponent::_lock()
	{
		xal::mgr->_lock();
	}

	void ExternalComponent::_unlock()
	{
		xal::mgr->_unlock();
	}

}
