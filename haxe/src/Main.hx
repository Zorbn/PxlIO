import haxe.Int32;

class Main {
	static function main() {
		var rnd = new PxlRnd("Rendering Pixels in Haxe", 640, 480, 320, 240);
		rnd.setBackgroundColor(0.4, 0.6, 0.9);

		var spriteBatch = rnd.createSpriteBatch("res/tiles.png", 1000);
		var sprite = new Sprite();
		sprite.texX = 0;
		sprite.texY = 0;
		sprite.width = sprite.height = sprite.texWidth = sprite.texHeight = 16;

		var playerX = 0.0;
		var playerY = 0.0;
		final playerSpeed = 64.0;

		while (rnd.pollEvents()) {
			var deltaTime = rnd.getDeltaTime();

			if (rnd.isKeyHeld(KeyCode.KeyA)) {
				playerX -= playerSpeed * deltaTime;
			}

			if (rnd.isKeyHeld(KeyCode.KeyD)) {
				playerX += playerSpeed * deltaTime;
			}

			if (rnd.isKeyHeld(KeyCode.KeyW)) {
				playerY += playerSpeed * deltaTime;
			}

			if (rnd.isKeyHeld(KeyCode.KeyS)) {
				playerY -= playerSpeed * deltaTime;
			}

			spriteBatch.clear();
			spriteBatch.add(playerX, playerY, 0, sprite);

			rnd.beginDrawing();
			rnd.drawSpriteBatch(spriteBatch);
			rnd.endDrawing();
		}
	}
}
