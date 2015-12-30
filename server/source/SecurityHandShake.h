#ifndef SecurityHandShake_H
#define SecurityHandShake_H

#include "CompileFeatures.h"

#if ENABLE_SECURE_HAND_SHAKE == 1

// If building a  DLL, be sure to tweak the CAT_EXPORT macro meaning
#if !defined(JACIE_HAS_STATIC_LIB) && defined(JACIE_HAS_DYNAMIC_LIB)
# define CAT_BUILD_DLL
#else
#define CAT_NEUTER_EXPORT
#endif
#define CAT_AUDIT
#include <cat/AllTunnel.hpp>
#endif

#endif  //__SECURITYHANDSHAKE_H__

