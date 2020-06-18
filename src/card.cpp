#include "card.h"

Card::Card(SCM data, QObject *parent)
    : QObject(parent)
    , faceDown(!(scm_is_true(SCM_CADDR(data))))
    , suit(static_cast<Suit>(scm_to_int(SCM_CADR(data))))
    , rank(static_cast<Rank>(scm_to_int(SCM_CAR(data))))
{
}

SCM Card::toSCM() const
{
    return scm_cons(scm_from_uint(rank),
                    scm_cons(scm_from_uint(suit),
                             scm_cons(SCM_BOOL(!faceDown),
                                      SCM_EOL)));
}
