function parse_input_events (path)
  events = {}
  f = io.open(path)
  for line in f:lines() do
    time, command, sensor, value = line:match("(%d+) (%w+) *(%w*) *(%d*)")
    if command == "trigger1" then
      table.insert(events, {command=command, time=time})
    end
  end
  return events
end

function tick(time)
  if time % 5000 == 300 then
    hosted_trigger1(time)
  end
  on, off = hosted_get_output()
  if on ~= 0 or off ~= 0 then
    print(on, off)
  end
end

function set_output(output, value)
  print(output)
  print(value)
end

function tfi_platform_init() 
  events = parse_input_events("events_in")
  print(events[5].command)
end
