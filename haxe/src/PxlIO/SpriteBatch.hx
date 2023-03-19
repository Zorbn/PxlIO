package pxlio;

import haxe.Int32;

class SpriteBatch {
	public final id:Int32;

	public function new(id:Int32) {
		this.id = id;
	}

	public function clear() {
		PxlIOBindings.pxlio_sprite_batch_clear(this.id);
	}

	public function add(x:Single, y:Single, z:Single, sprite:Sprite) {
		PxlIOBindings.pxlio_sprite_batch_add(id, x, y, z, sprite.width, sprite.height, sprite.texX, sprite.texY, sprite.texWidth, sprite.texHeight,
			sprite.originX, sprite.originY, sprite.rotation, sprite.r, sprite.g, sprite.b, sprite.a, sprite.tint);
	}
}
