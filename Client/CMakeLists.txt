include(App)
include(Base)
include(Network)
include(Rendering)
include(ClientShared)

add_executable(Player
   ${CMAKE_SOURCE_DIR}/ClientShared/SDLGameController.cpp
   ${CMAKE_SOURCE_DIR}/ClientShared/SDLGameController.h
   ${CMAKE_SOURCE_DIR}/ClientBase/MachineConfiguration.cpp
   ${CMAKE_SOURCE_DIR}/ClientBase/MachineConfiguration.h
   ${CMAKE_SOURCE_DIR}/ClientBase/RenderSettingsItem.cpp
   ${CMAKE_SOURCE_DIR}/ClientBase/RenderSettingsItem.h

   src/main.cpp
   src/Core.cpp
   src/Core.h
   src/Application.cpp
   src/Application.h
   src/FunctionMarshaller.cpp
   src/FunctionMarshaller.h
   src/View.cpp
   src/View.h
   src/RenderJob.cpp
   src/RenderJob.h
)

target_link_libraries(Player PRIVATE
   $<TARGET_OBJECTS:Base>
   $<TARGET_OBJECTS:Log>
   $<TARGET_OBJECTS:App>
   $<TARGET_OBJECTS:Network>
   $<TARGET_OBJECTS:G3D>
   $<TARGET_OBJECTS:AppDraw>
   $<TARGET_OBJECTS:RbxG3D>
   $<TARGET_OBJECTS:GfxBase>
   $<TARGET_OBJECTS:GfxCore>
   $<TARGET_OBJECTS:GfxRender>
   $<TARGET_OBJECTS:BulletPhysics>
   ${DEPENDENCIES}
)