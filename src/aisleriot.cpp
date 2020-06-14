#include <libguile.h>
#include "aisleriot.h"

Aisleriot::Aisleriot(QObject *parent)
    : QObject(parent)
{
    scm_c_define_module("aisleriot interface", interface_init, this);
}

Aisleriot::~Aisleriot()
{
}

void Aisleriot::interface_init(void *data)
{
    // See aisleriot/src/game.c:cscm_init for reference
    Aisleriot *game = static_cast<Aisleriot*>(data);
    /* TODO: List all C functions that Aisleriot can call */
}
