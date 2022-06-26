require("entities")
require("math")

PlayerController = {}

math.randomseed(123456)

local velocity = glm.vec3(0)
local acceleration = glm.vec3(0)

function PlayerController:OnStart()
    print("player controller startup...")
end

function random2D()
    v = glm.vec3(0)
    x = math.random(1,100)
    y = math.random(1,100)
    if x > 50 then
        v.x = 1.
    else
        v.x = -1.
    end

    if y < 50 then
        v.z = 1.
    else
        v.z = -1.
    end

    return v
end

local lost_time = 0.0
local dir = glm.vec3(0,0,1)

function PlayerController:OnUpdate(delta)
    scalar = math.random() * delta * 0.05

    if lost_time > 1.0 then
        dir = random2D()
        lost_time = 0.0
    end

    acceleration.x = math.random() * dir.x
    acceleration.z = math.random() * dir.z
    print("acc1",tostring(acceleration))

    acceleration = glm.vec3.normalize(acceleration) * scalar
    print("acc2",tostring(acceleration))

    velocity = velocity + acceleration

    velocity = glm.vec3.clamp(velocity,glm.vec3(-0.1),glm.vec3(0.1))

    print("vel",tostring(velocity))

    transform = PlayerController.transform
    transform.position = transform.position + velocity
    transform:rotate(velocity)

    lost_time = lost_time + delta
    print("walking...",lost_time)
end

function PlayerController:OnExit()
    print("Disconnected ...")
end

return PlayerController