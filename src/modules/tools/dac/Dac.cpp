/*
    This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
    Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "modules/communication/utils/Gcode.h"
#include "modules/robot/Stepper.h"
#include "modules/robot/Conveyor.h"
#include "Dac.h"
#include "libs/nuts_bolts.h"
#include "Config.h"
#include "StreamOutputPool.h"
#include "Block.h"
#include "checksumm.h"
#include "ConfigValue.h"

#include "libs/Pin.h"
#include "Gcode.h"
#include "AnalogOut.h" // mbed.h lib

#define dac_module_enable_checksum          CHECKSUM("dac_module_enable")

Dac::Dac(){
}

void Dac::on_module_loaded() {
    if( !THEKERNEL->config->value( dac_module_enable_checksum )->by_default(false)->as_bool() ){
        // as not needed free up resource
        delete this;
        return;
    }
    this->dac_pin = new mbed::AnalogOut(p18);

    this->dac_value = 0.0f;
    this->dac_pin->write(this->dac_value);

    //register for events
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_GCODE_EXECUTE);
    // this->register_for_event(ON_PAUSE);      // Alex: not sure if this is neccessary
    // this->register_for_event(ON_HALT);       // Alex: not sure if this is neccessary
}

void Dac::on_gcode_received(void * argument){
    Gcode* gcode = static_cast<Gcode*>(argument);
    if( gcode->has_m && (gcode->m == 25) ){
        // Pass gcode along to on_gcode_execute
        THEKERNEL->conveyor->append_gcode(gcode);
        gcode->mark_as_taken();
    }
}

// set dac depending on received GCodes
void Dac::on_gcode_execute(void* argument){
    Gcode* gcode = static_cast<Gcode*>(argument);
    if( gcode->has_m && (gcode->m == 25) ){
        if( gcode->has_letter('P') ){
            this->dac_value = min(1.0f, float(gcode->get_value('P')));
        } else {
            this->dac_value = 0.0f;
        }
        this->dac_pin->write(dac_value);
        THEKERNEL->streams->printf("Adjusted dac to %d (of 1024)\r\n",(int)(this->dac_value*1024+0.5));
    }
}

