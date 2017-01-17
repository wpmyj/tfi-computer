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
  if time % 1000 == 800 then
    hosted_trigger1(time)
  end
end

function tfi_platform_init() 
  events = parse_input_events("events_in")
  print(events[5].command)
end
