#!/usr/bin/ruby

include Math

def resistanceToC(r)
  b=4100.0
  r0=10.0
  t0=25.0+273.15

  kelvin=b/Math.log(r/(r0*E**(-b/t0)))
  return kelvin-273.15
end

def voltageToRef(v)
  vref=1.1
  r0=10.0

  return (vref-v*r0+r0)/v
end

print resistanceToC(34.336),"\n"
print resistanceToC(10),"\n"
print resistanceToC(5.187),"\n"
print resistanceToC(1.148),"\n"

print resistanceToC(voltageToRef(0.55)),"\n"
