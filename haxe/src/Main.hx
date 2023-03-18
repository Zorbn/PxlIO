class Main {
	static function main() {
		var rnd = new PxlRnd("Rendering Pixels in Haxe", 640, 480, 320, 240);
		rnd.setBackgroundColor(0.4, 0.6, 0.9);

		var spriteBatch = rnd.createSpriteBatch("res/tiles.png", 1000);
		var sprite = new Sprite();
		sprite.texX = 0;
		sprite.texY = 0;
		sprite.width = sprite.height = sprite.texWidth = sprite.texHeight = 16;

		while (rnd.pollEvents()) {
			var deltaTime = rnd.getDeltaTime();

			spriteBatch.clear();
			spriteBatch.add(64, 64, 0, sprite);

			rnd.beginDrawing();
			rnd.drawSpriteBatch(spriteBatch);
			rnd.endDrawing();
		}
	}
}
