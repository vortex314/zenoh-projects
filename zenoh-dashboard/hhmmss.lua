function process_data(data)
  local v = data
  local msec = v % 1000
  local sec = ( v // 1000 ) %60
  local mins = ( v // 60000) % 60 
  local hours = ( v // ( 3600 * 1000))
  return string.format("%d:%2.2d:%2.2d.%3.3d",hours,mins,sec,msec)
end

