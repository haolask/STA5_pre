///
//! \file          presets.cpp
//! \brief         Class used to manage radio presets list
//! \author        Alberto Saviotti
//!
//! Project        STRaDe
//! Sw component   HMI
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20180130       | Initial version            | Alberto Saviotti
///

#include "presets.h"

Presets *Presets::presets = nullptr;

Presets * Presets::Instance()
{
    if (nullptr == presets)
    {
        presets = new Presets();
    }

    return presets;
}

// End of file
