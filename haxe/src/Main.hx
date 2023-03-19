import haxe.Int32;

class Main {
	static function main() {
		var io = new PxlIO("Rendering Pixels in Haxe", 640, 480, 320, 240);
		io.setBackgroundColor(0.4, 0.6, 0.9);

		var spriteBatch = io.createSpriteBatch("res/tiles.png", 1000);
		var sprite = new Sprite();
		sprite.texX = 0;
		sprite.texY = 0;
		sprite.width = sprite.height = sprite.texWidth = sprite.texHeight = 16;

		var playerX = 0.0;
		var playerY = 0.0;
		final playerSpeed = 64.0;

		var audio = new Audio("res/explosion.wav");

		while (io.pollEvents()) {
			var deltaTime = io.getDeltaTime();

			if (io.wasKeyPressed(KeyCode.Escape)) {
				io.close();
			}

			if (io.isKeyHeld(KeyCode.A)) {
				audio.play();
				playerX -= playerSpeed * deltaTime;
			}

			if (io.isKeyHeld(KeyCode.D)) {
				playerX += playerSpeed * deltaTime;
			}

			if (io.isKeyHeld(KeyCode.W)) {
				playerY += playerSpeed * deltaTime;
			}

			if (io.isKeyHeld(KeyCode.S)) {
				playerY -= playerSpeed * deltaTime;
			}

			spriteBatch.clear();
			spriteBatch.add(playerX, playerY, 0, sprite);

			io.beginDrawing();
			io.drawSpriteBatch(spriteBatch);
			io.endDrawing();
		}
	}
}
