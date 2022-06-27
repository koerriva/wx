Enemy = {entity=0}

function Enemy:OnStart()
    print("enemy controller startup...")
end

function Enemy:OnUpdate(delta)
    --print("enemy",Enemy.entity)
    print("delta",delta)
end

function Enemy:OnExit()

end

return Enemy