Enemy = {entity=0}

function Enemy:OnStart()
    print("enemy controller startup...")
end

function Enemy:OnUpdate(delta)
    --print("enemy",Enemy.entity)
end

function Enemy:OnExit()

end

return Enemy