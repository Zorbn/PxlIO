class Main {
	static function main() {
		trace("Hello from haxe!");
		trace(PxlRnd.get_int(777));

		PxlRnd.pxlrnd_create("Hello, World!", 640, 480, 320, 240, true);
		PxlRnd.pxlrnd_set_background_color(0, 0, 0.2);
		PxlRnd.pxlrnd_set_screen_background_color(1, 1, 1);

		var spriteBatch = PxlRnd.pxlrnd_create_sprite_batch("res/tiles.png", 50000, false, false);

        while (PxlRnd.pxlrnd_poll_events()) {
			var deltaTime = PxlRnd.pxlrnd_get_delta_time();
			// trace(deltaTime);

			PxlRnd.pxlrnd_begin_drawing();
			PxlRnd.pxlrnd_sprite_batch_clear(spriteBatch);

			for (i in 0...50000) {
				PxlRnd.pxlrnd_sprite_batch_add(spriteBatch, 0, 0, 0, 64, 64, 0, 0, 64, 64, 0, 0, 0, 1, 1, 1, 1, 0);
			}

			PxlRnd.pxlrnd_draw_sprite_batch(spriteBatch);
			PxlRnd.pxlrnd_end_drawing();
		}

		PxlRnd.pxlrnd_destroy_sprite_batch(spriteBatch);
		trace("Goodbye from haxe!");
	}
}
