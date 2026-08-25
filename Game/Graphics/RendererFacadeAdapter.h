#pragma once

// Compatibilidade temporária durante o cut-over final.
// A implementação vive inteiramente em RendererFacade; não existe adapter
// adicional nem estado de renderização duplicado.
#include "Graphics/RendererFacade.h"

namespace gfx {
using RendererFacadeAdapter = RendererFacade;
}
