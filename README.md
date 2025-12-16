# RSGO

Minimalistic Quake-Inspired Online Multiplayer Shooter made in C++ using OpenGL and minimal dependencies. Network model is implemented through raw sockets and follows a non-authoritative replication model where all input is trusted.

### AUTHORS: midtxwn & kentalives @ Github
Original creation date: 16/05/2024
# LAUNCHING THE GAME: 
Clone the repo, enter /RSGO/ folder, run RSGO.exe in terminal. Once launched, the console will ask for a series of options.

You can run it in LOCAL MODE (2) to test it without connecting to any other player,
or you can choose a client mode and a server mode to establish multiplayer connections.

Currently, the game only supports 2-player games.

## WEAPONS:
The game has 2 weapons, a knife and a rifle. The rifle has semi-automatic fire with
infinite ammunition. The knife allows you to move at a higher speed.

To select the knife, press 2.
To select the rifle, press 1.

## MOVEMENT:
The player can move in all directions and can crouch with LEFT CONTROL.

The player will move with the WASD keys.

## MODELS:
We primarily divide the work into the map itself, the local player, and the opposing player, each of which is a .obj file that we parse at the beginning of the program using a custom class. The local player effectively has two different models that alternate when changing weapons (one per weapon), while the opposing player has a unified model whose legs swap when crouching, in addition to reducing its height.
Regarding the management of these obtained objects, two categories are distinguished: entities, which are groupings of objects that conceptually form a unit. These are the ones referred to at the beginning of this section and are obtained from a complete .obj file. The other division is called "components," which refers to each of the individual objects (taken from Blender) that make up the entities. These are a set stored in the structure previously referred to as "entity" and are obtained from the .obj file corresponding to the entity that contains them. This division allows for the manipulation of the conceptual grouping as a whole
and the fine-grained manipulation of each of its components individually.

## CAMERA:
Players will see the world in first person based on their character, and this camera moves based
on the mouse movement obtained as "raw input," rotating the character horizontally based on their
horizontal movement and rotating the character's head vertically also based on the mouse.

Additionally, using the 'C' key, you can switch to a "legacy" camera from above. In this view, you can see
the first-person model, which will be discussed later for reasons explained further below.

## COLLISIONS:
For keyboard movement, collisions with the various map elements are checked
along each of the object axis, as they are all aligned with the modeling axis.

When firing, collisions with any component of the opposing player are checked using raycasting
to determine if the shot hit its target.

## ONLINE:
For synchronization across the network, standard packets containing the transformation matrix, position, and rotation angle associated with the vertical head movement are periodically sent.
This represents the opposing player in the final position obtained from the other player via the network. This periodic transmission, reception, and update work is handled by a pair of dedicated threads to avoid overloading the main thread.
In addition, for asynchronous events such as weapon changes, crouching, or firing, specific packets are created to represent the event at the moment it occurs. These are analyzed upon receipt and processed from the main loop. In contrast to periodic messages, event-related messages are sent by the main thread, as it detects the events, and subsequently received and managed by the dedicated reception thread, which also handles periodic TCP management.
Managing TCP messages using dedicated threads presents some challenges, which are addressed in the "KNOWN ISSUES" section.

## EFFECTS:
When the rifle is fired, a recoil effect occurs, and a square is drawn
at the end of the weapon. This square is filled with a smoke texture that changes briefly after the shot before disappearing. Additionally, if the player is shot by another player, a texture is applied to
the local player's view, indicating that a bullet impact was taken, reddening the edge of the screen.

## HUD:
To always represent the first-person view with arms and weapon without them passing through objects, and to adapt to camera movement with the mouse, the elements of the first-person model
never move from their origin. They are drawn on top of everything else, and the lighting changes on them are achieved
by simulating the movement of light before drawing them, instead of using the actual position of the light, thus creating
a movement effect relative to the light.

## GAMEPLAY:
Each player has 10 lives. When hit by a bullet, one is subtracted.

# KNOWN ISSUES/FUTURE FUNCTIONALITY TO IMPLEMENT:
The following are some known issues that have not been resolved/implemented.

It would be beneficial to address them if the game's functionality continues to be extended.

### CRITICAL RACES:

When using threads, it would be necessary to implement synchronization mechanisms (mutexes or locks)
when writing, or implement a double-buffering policy to avoid lost writes.

Therefore, the game can sometimes become unsynchronized between clients (for example,
one player might see that another has the knife when they actually have the weapon selected).

### WALLBANG:

The raycast function for detecting enemy impacts does not take into account collisions with map elements.

This has a simple solution: it would be necessary to check if there is a vector with a smaller magnitude
with any map element compared to the vector that impacts the player.

### COLLISION IMPROVEMENT:

It would be necessary to implement a system where, in the event of a collision, instead of setting the player's velocity
to 0, the velocity vector is projected onto a vector parallel to the surface of the collision,
allowing the player to "slide" upon impact with a surface at high speed.

### SHOOTING IMPROVEMENT:

A simple but common addition in these games would be differentiating between headshots and hits
on other parts of the opponent's model. This would be quite easy to implement, as we already detect the component
of the hit using raycasting. Furthermore, it would be beneficial to include a visual metric that confirms
the hit on an opponent from the attacker's perspective, such as a "hitmarker."

# CREDITS:

All code and models (excluding textures) were created by the authors.

The game is inspired by Quake's movement; it would be interesting to add jumping mechanics,a more robust friction system, and acceleration similar to Quake's engine to be able to enjoy advanced movement mechanics like bunny hopping.

To recreate part of Quake's movement, we relied on the following paper:

https://github.com/myria666/qMovementDoc/blob/main/Quake_s_Player_Movement.pdf
