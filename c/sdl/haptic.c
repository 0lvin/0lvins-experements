#include <SDL2/SDL.h>

int main() {
	SDL_Joystick * joystick;
	if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC))
	{
		if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != -1)
		{
			if (SDL_NumJoysticks() > 0) {
				int i;
				for (i=0; i<SDL_NumJoysticks(); i ++) {
					joystick = SDL_JoystickOpen(i);
					SDL_Haptic *haptic;
					SDL_HapticEffect effect;
					int effect_id;

					// Open the device
					haptic = SDL_HapticOpenFromJoystick( joystick );
					if (haptic == NULL) return -1; // Most likely joystick isn't haptic

					// See if it can do sine waves
					if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE)==0) {
						SDL_HapticClose(haptic); // No sine effect
						return -1;
					}

					for(int j = 0; j < 36000; j += 9000) {
						printf("d:%d\n", j);
						// Create the effect
						SDL_memset( &effect, 0, sizeof(SDL_HapticEffect) ); // 0 is safe default
						effect.type = SDL_HAPTIC_SINE;
						effect.periodic.direction.type = SDL_HAPTIC_POLAR; // Polar coordinates
						effect.periodic.direction.dir[0] = j; // Force comes from south
						effect.periodic.period = 1000; // 1000 ms
						effect.periodic.magnitude = 10000; // 10000/32767 strength
						effect.periodic.length = 4000; // 5 seconds long
						effect.periodic.attack_length = 500; // Takes 1 second to get max strength
						effect.periodic.fade_length = 500; // Takes 1 second to fade away

						// Upload the effect
						effect_id = SDL_HapticNewEffect( haptic, &effect );

						// Test the effect
						SDL_HapticRunEffect( haptic, effect_id, 1 );
						SDL_Delay( 5000); // Wait for the effect to finish

						// We destroy the effect, although closing the device also does this
						SDL_HapticDestroyEffect( haptic, effect_id );
					}

					// Close the device
					SDL_HapticClose(haptic);

					return 0; // Success

				}
			}
		}
	}
}
