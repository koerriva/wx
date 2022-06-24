local times = 20

function OnStart()
    print("player controller startup...")
    times = 2;
end

function OnUpdate(delta)
    times = times + 1
--     print(times)
end