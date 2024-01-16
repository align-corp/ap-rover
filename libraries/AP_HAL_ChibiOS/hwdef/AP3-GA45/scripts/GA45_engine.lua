-- emergency stop and starter for Align GA45

-- user parameters
local MILLIS_UPDATE = 100
local START_PIN = 57 -- AP3 ch 8
local IGNITION_PIN = 59 -- AP3 ch 10

-- parameters
local PARAM_TABLE_KEY = 41
assert(param:add_table(PARAM_TABLE_KEY, "ENG_", 4), "could not add param table")
assert(param:add_param(PARAM_TABLE_KEY, 1, "DEBUG", 0), "could not add param 1")
assert(param:add_param(PARAM_TABLE_KEY, 2, "CH", 6), "could not replace param 2")
assert(param:add_param(PARAM_TABLE_KEY, 3, "REV_IGN", 1), "could not replace param 3")
assert(param:add_param(PARAM_TABLE_KEY, 4, "REV_STR", 0), "could not replace param 4")

-- bind parameters to variables
local ENG_DEBUG = Parameter()
local ENG_REV_IGN = Parameter()
local ENG_REV_STR = Parameter()
local ENG_CH = Parameter()
ENG_DEBUG:init("ENG_DEBUG")
ENG_REV_IGN:init("ENG_REV_IGN")
ENG_REV_STR:init("ENG_REV_STR")
ENG_CH:init("ENG_CH")

-- global variables and init
local _eng_ch = 0
local _starter_first = true
local ignition_on = 1
local ignition_off = 0
local start_on = 1
local start_off = 0
gpio:pinMode(IGNITION_PIN,1) -- set ignition pin as output
gpio:write(IGNITION_PIN, ignition_off) -- set ignition off
gpio:pinMode(START_PIN,1) -- set ignition pin as output
gpio:write(START_PIN, start_off) -- set starter pin off

function check_channels_params()
    _eng_ch = ENG_CH:get()
    if _eng_ch < 4 and _eng_ch > 16 then
        gcs:send_text('3', "Select the RC channel for engine stop through param ENG_CH")
        return check_channels_params, 5000
    end

    return update, MILLIS_UPDATE
end

function set_ignition()
    if rc:get_pwm(_eng_ch) > 1350 then
        gpio:write(IGNITION_PIN, ignition_on)
        if ENG_DEBUG:get() > 1 then
            gcs:send_text('6', "Ignition ON")
        end
    else
        gpio:write(IGNITION_PIN, ignition_off)
        if ENG_DEBUG:get() > 1 then
            gcs:send_text('6', "Ignition OFF")
        end
    end
end

function set_starter()
    if _starter_first then
        gpio:write(START_PIN, start_off)
        if rc:get_pwm(_eng_ch) < 1800 then
            _starter_first = false
        end
    elseif rc:get_pwm(_eng_ch) > 1850 then
        gpio:write(START_PIN, start_on)
        if ENG_DEBUG:get() > 1 then
            gcs:send_text('6', "Starter ON")
        end
    else
        gpio:write(START_PIN, start_off)
        if ENG_DEBUG:get() > 1 then
            gcs:send_text('6', "Starter OFF")
        end
    end
end

function update()
    set_ignition()
    set_starter()
    return update, MILLIS_UPDATE
end

-- check reverse parameters
if ENG_REV_IGN:get() > 0 then
    ignition_on = 0
    ignition_off = 1
end
if ENG_REV_STR:get() > 0 then
    start_on = 0
    start_off = 1
end

return check_channels_params, 1000