set(_AVAILABLE_OPTIONS "")

macro(ADD_OPTION _name _desc _value)
  if (DEFINED ${_name})
    set(${_name}_ALREADY_DEFINED ${${_name}})
  endif()
  option(${_name} ${_desc} ${_value})
  set(${_name}_VALUE ${_value})
  list(APPEND _AVAILABLE_OPTIONS ${_name})
endmacro()

macro(OVERRIDE_DEFAULT _name _value)
  set(${_name}_OVERRIDE_DEFAULT ${_value})
endmacro()

function(PROCESS_OPTIONS)
  message("-- Option list:")
  foreach(_name ${_AVAILABLE_OPTIONS})
    if (DEFINED ${_name}_ALREADY_DEFINED)
      message("   - ${_name}: ${${_name}}")
    elseif (DEFINED ${_name}_OVERRIDE_DEFAULT)
      message("   - ${_name}: ${${_name}_OVERRIDE_DEFAULT}")
    elseif (DEFINED ${_name}_VALUE)
      message("   - ${_name}: ${${_name}_VALUE}")
    else()
      message("   - ${_name}: ${${_name}}")
    endif()

    if (DEFINED ${_name}_ALREADY_DEFINED)
      if (${${_name}_ALREADY_DEFINED})
        add_definitions("-D${_name}=${${_name}_ALREADY_DEFINED}")
      endif()
    elseif (${${_name}_OVERRIDE_DEFAULT})
      add_definitions("-D${_name}=${${_name}_OVERRIDE_DEFAULT}")
    elseif (${${_name}})
      add_definitions("-D${_name}=${${_name}_VALUE}")
    endif()
  endforeach()
endfunction()
