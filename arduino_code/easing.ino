// EASING_FUNCTIONS:
// no easing, no acceleration
uint8_t easing_linear(uint8_t t) { return t; }
// accelerating from zero velocity
uint8_t easing_easeInQuad(uint8_t t) { return t * t; }
// decelerating to zero velocity
uint8_t easing_easeOutQuad(uint8_t t) { return t * (2 - t); }
// acceleration until halfway, then deceleration
uint8_t easing_easeInOutQuad(uint8_t t) { return t < .5 ? 2 * t * t : -1 + (4 - 2 * t) * t; }
// accelerating from zero velocity
uint8_t easing_easeInCubic(uint8_t t) { return t * t * t; }
// decelerating to zero velocity
uint8_t easing_easeOutCubic(uint8_t t) { return (--t) * t * t + 1; }
// acceleration until halfway, then deceleration
uint8_t easing_easeInOutCubic(uint8_t t) { return t < .5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1; }
// accelerating from zero velocity
uint8_t easing_easeInQuart(uint8_t t) { return t * t * t * t; }
// decelerating to zero velocity
uint8_t easing_easeOutQuart(uint8_t t) { return 1 - (--t) * t * t * t; }
// acceleration until halfway, then deceleration
uint8_t easing_easeInOutQuart(uint8_t t) { return t < .5 ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t; }
// accelerating from zero velocity
uint8_t easing_easeInQuint(uint8_t t) { return t * t * t * t * t; }
// decelerating to zero velocity
uint8_t easing_easeOutQuint(uint8_t t) { return 1 + (--t) * t * t * t * t; }
// acceleration until halfway, then deceleration
uint8_t easing_easeInOutQuint(uint8_t t) { return t < .5 ? 16 * t * t * t * t * t : 1 + 16 * (--t) * t * t * t * t; }