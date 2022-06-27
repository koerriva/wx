require("math")

Enemy = {entity=0}

math.randomseed(123)

local velocity = glm.vec3(0)
local acceleration = glm.vec3(0)
local target_position = glm.vec3(0)
local lost_time = 0.

function Enemy:OnStart()
    print("enemy controller startup...")
end

function Enemy:OnUpdate(delta)
    transform = Enemy.transform

    target_position.x = PlayerController.transform.position.x+1
    target_position.z = PlayerController.transform.position.z+1
    target_position.y = transform.position.y
    print("target_position",target_position)

    dir = target_position-transform.position
    dir = glm.vec3.normalize(dir)
    print("dir",dir)

    acceleration = dir * delta * 0.01
    velocity = velocity + acceleration
    velocity = glm.vec3.clamp(velocity,glm.vec3(-0.1),glm.vec3(0.1))

    print("velocity",velocity)

    transform.position = transform.position + velocity
    transform:rotate(dir)

    lost_time = lost_time + delta
    print("delta",delta)
end

function Enemy:OnExit()

end

return Enemy