#include "ActionFactory.h"
#include "Action.h"

using pAction = ActionFactory::pAction;

LinearMovement ActionFactory::createLinearMovement() {
    return LinearMovement();
}

CircularMovement ActionFactory::createCircularMovement() {
    return CircularMovement();
}

EllipticalMovement ActionFactory::createEllipticalMovement() {
    return EllipticalMovement();
}

pAction ActionFactory::createAwait(double awaitTime) {
    return std::make_unique<Await>(awaitTime);
}
