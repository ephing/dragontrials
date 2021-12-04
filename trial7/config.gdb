set breakpoint pending on
set confirm off
file ./cshantyc
break cshanty::Err::report
commands
	where
end
break cshanty::ToDoError::ToDoError
commands
	where
end
break cshanty::InternalError::InternalError
commands
	where
end

define t7
  set args p6_tests/$arg0.cshanty -o --
  run
end
