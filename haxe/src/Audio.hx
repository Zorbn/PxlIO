import haxe.Int32;

class Audio {
    private var id:Int32;

    public function new(path:String) {
        id = PxlIOBindings.pxlio_audio_constructor(path);
    }

    public function setVolume(volume:Single) {
        PxlIOBindings.pxlio_audio_set_volume(id, volume);
    }

    public function play() {
        PxlIOBindings.pxlio_audio_play(id);
    }

    public function destroy() {
        PxlIOBindings.pxlio_audio_destroy(id);
    }
}