import haxe.Int32;

@:hlNative("PxlRnd")
class PxlRndBindings {
	public static function pxlrnd_create(windowName:String, windowWidth:Int32, windowHeight:Int32, viewWidth:Int32, viewHeight:Int32, enableVsync:Bool):Void {}

	public static function pxlrnd_poll_events():Bool {
		return false;
	}

	public static function pxlrnd_get_delta_time():Single {
		return 0.0;
	}

	public static function pxlrnd_begin_drawing():Void {}

	public static function pxlrnd_end_drawing():Void {}

	public static function pxlrnd_set_background_color(r:Single, g:Single, b:Single) {}

	public static function pxlrnd_set_screen_background_color(r:Single, g:Single, b:Single) {}

	public static function pxlrnd_create_sprite_batch(texturePath:String, maxSprites:Int32, smooth:Bool, enableBlending:Bool):Int32 {
		return 0;
	}

	public static function pxlrnd_destroy_sprite_batch(id:Int32) {}

	public static function pxlrnd_sprite_batch_clear(id:Int32) {}

	public static function pxlrnd_sprite_batch_add(id:Int32, x:Single, y:Single, z:Single, width:Single, height:Single, texX:Single, texY:Single,
		texWidth:Single, texHeight:Single, originX:Single, originY:Single, rotation:Single, r:Single, g:Single, b:Single, a:Single, tint:Single) {}

	public static function pxlrnd_draw_sprite_batch(id:Int32) {}

	public static function pxlrnd_is_key_held(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_was_key_pressed(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_was_key_released(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_get_pressed_keys():hl.Bytes {
		return null;
	}

	public static function pxlrnd_get_mouse_x():Int32 {
		return 0;
	}

	public static function pxlrnd_get_mouse_y():Int32 {
		return 0;
	}

	public static function pxlrnd_is_mouse_button_held(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_was_mouse_button_pressed(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_was_mouse_button_released(keyNumber:Int32):Bool {
		return false;
	}

	public static function pxlrnd_close() {}
}
