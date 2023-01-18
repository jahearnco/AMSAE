# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
	"main.cpp" 
  "AMSAESingleton.h"
  "AMSAESingleton.cpp"
	"Idler.h"
	"Idler.cpp" 
	"TaskManager.h" 
	"TaskManager.cpp" 
	"LoopManager.h" 
	"LoopManager.cpp" 
	"KineticsManager.h" 
	"KineticsManager.cpp" 
	"SerialManager.h" 
	"SerialManager.cpp" 
	"TimingManager.h" 
	"TimingManager.cpp" 
	"PositionSensor.h" 
  "PositionSensor.cpp" 
	"PulseCycleManager.h" 
	"PulseCycleManager.cpp" 
	"PulseCycle.h" 
	"PulseCycle.cpp" 
	"Pulse.h" 
	"Pulse.cpp" 
	"Coil.h" 
	"Coil.cpp" 
	"CoilContainer.h" 
	"CoilContainer.cpp" 
	"Brain.h" 
	"Brain.cpp" 
	"KineticProfileTickHistory.h" 
	"KineticProfileTickHistory.cpp" 
	"KineticProfileTickHistoryContainer.h" 
	"KineticProfileTickHistoryContainer.cpp" 
	"KineticPulseProfile.h"
	"AMSAEUtils.h" 
	"AMSAEUtils.cpp" 
	)
set(COMPONENT_ADD_INCLUDEDIRS 
	"."
	)
register_component()