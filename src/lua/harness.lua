TriggerWheel = {}
function TriggerWheel:new (o) 
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  self.times = {}
  self.speeds = {}
  return o
end

function TriggerWheel:set_rpm (time, rpm)
  table.insert(self.times, time)
  self.speeds[time] = rpm
end

function TriggerWheel:get_outputs (time)
  return self:get_angle(time) % (360 / self.primary_teeth) == 0
end

function degrees_from_rpm (rpm, curtime)
  return 6 * rpm * curtime
  -- 6 degrees per second per rpm
end

function TriggerWheel:get_angle (time)
  local curtime = 0
  local last_angle = 0
  for i, t in ipairs(self.times) do
    if time >= curtime and time < t then
      angle = last_angle + degrees_from_rpm(self.speeds[t], time - curtime)
      return (angle % 360)
    end
    next_angle = last_angle + degrees_from_rpm(self.speeds[t], t - curtime)
    curtime = t
    last_angle = next_angle % 360
  end
  return (last_angle + degrees_from_rpm(self.speeds[self.times[#self.times]], time - curtime)) % 360
end


Validator = {}
function Validator:new (o) 
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end


Harness = {}
function Harness:new (o) 
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Harness:set_analog (index, value)

end

