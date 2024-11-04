Link to video: https://youtu.be/nNZPoSXdvME

This project aims to innovate movement mechanics within first-person shooter (FPS) games, utilizing Unreal Engine and C++. By introducing advanced mechanics such as bunny hopping, air control, surface sliding, nd surfing. I seek to enhance offer a liberating sense of freedom, allow players to be and feel fast, include movement as a mechanic that can be taken advantage of, and put a light of potential spatial awareness skills that could be developed by playing games with advanced unlinear movement systems.

### Bunny Hopping
Allows players to gain speed and maintain momentum, what requires a bit of skill to help type in console move.AutoBhop 1
BunnyHopping is achievable by walking in forward direction press W, then jumping. Whilst in air utilize only A or D to move left or right and match direction with fluid movement of the mouse in the same direction to increase speed. When approaching Landing, press jump again only using A or D input.
Extra things are that you can see the logic that can make character exceed max ground speed. It can be done by pressing and holding W+A or W+D to make character go diagonally.

### Air Control
Provides players with mid-air maneuverability.
Whilst in air, you can active make character change direction gain acceleration. Acceleration is increase because character is falling, for that I have overrided CalcVelocity method. Secondly fluid direction change without losing speed can only happen if we utilize A or D move whilst matching it with smooth mouse movement in desired direction. On top of that there is this mechanic called strafing. It works when you quickly change the movement in air from left to right whilst matching it with smooth mouse movement. 

### Surfing
Allows characters to glide off angled surfaces to generate or keep the momentum. It works on the same basis. Whilst being on the angled surface, player character can go in the straight line or go down the ramp by holding A or D key(if the ramp is one the left side hold A, if on right hold D). Proper surfing is one potential aspect that I could improve and implement into this existing system.



## Overview
Game will be  is a first-person shooter(FPS)/ Puzzle game with a heavy focus on advanced movement mechanics. 
The game is designed to be implemented in Unreal Engine 5, using both C++ and Blueprints to create a rich, immersive experience.

Inspirations/Visualization/ (not including the art part as I will use only free assests available on Epic Marketplace)
GhostRunner 1 / 2 - https://www.youtube.com/watch?v=Hgah6hvpo6A 
Cyberpunk 2077 - https://www.youtube.com/watch?v=aceFr4K9D-Y
Mirror's Edge - https://www.youtube.com/watch?v=UwDp-vNgRBY
Counter Strike : Source - https://www.youtube.com/watch?v=UxQ0x-AiJOY


# Feasibility
Given the two-month development timeframe, the initial focus will be on creating a solid prototype featuring core movement mechanics and basic AI interactions. This phase is critical for establishing the gameplay foundation and ensuring the project's feasibility.

# Technical Depth 
The project will leverage Unreal Engine 5's advanced features, Including Physics and Animation systems (NTH - Lumen Lightining?), some form of AI. Developing custom C++ components for movement and AI will showcase and improve my programming skills.

# Complex Control System 
The advanced movement mechanics demand a complex control system, warranting investigation into effective human-computer interaction methods. This aspect will involve refining input responsiveness and intuitiveness through user testing.

# Analytics Tracking 
Analytics will be integrated to track key gameplay features, such as player movement patterns(I wonder if I can store that data somehow via navmesh), weapon usage(Using the Gun/Sword Scenario). This data will inform iterative improvements and balancing.



# UE5 Stucture & Organisation

1. [UE5 Directory Structure](https://docs.unrealengine.com/5.1/en-US/unreal-engine-directory-structure/)
2. [UE5 Asset Naming Conventions](https://docs.unrealengine.com/5.1/en-US/recommended-asset-naming-conventions-in-unreal-engine-projects/)


# Other Useful Links

1. [UE5 Command-Line Arguments](https://docs.unrealengine.com/latest/en-US/command-line-arguments-in-unreal-engine/)
2. [C++ UPROPERTY Specifiers](https://docs.unrealengine.com/latest/en-US/unreal-engine-uproperties/)
3. [C++ UFUNCTION Specifiers](https://docs.unrealengine.com/latest/en-US/ufunctions-in-unreal-engine/)
4. [C++ UCLASS Specifiers](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/GameplayArchitecture/Classes/)
5. [C++ API Reference](https://docs.unrealengine.com/latest/en-US/API/)