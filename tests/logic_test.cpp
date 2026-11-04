#include <cassert>
#include "logic.hpp"
int main(){ assert(qol::new_death(true,0)); assert(!qol::new_death(false,0)); assert(!qol::new_death(true,2)); }
