#!/usr/bin/php
<?php

$midgard = new midgard_connection();
$midgard->open("midgard");
$midgard_config = new midgard_config();
$midgard_config->read_file("midgard");
midgard_user::auth($midgard_config->midgardusername,$midgard_config->midgardpassword);

class callback_class
{
    public static function my_callback($object, $data = null) 
    {
        echo "Connected to ".$object->guid." ".get_class($object)."\n";
        //echo "And have special data: $data1\n\n";
        return true;
    }
}

var_dump(is_callable(array('\\callback_class', "my_callback")));

midgard_object_class::connect_default("midgard_article", "action-update", array('\\callback_class', "my_callback"), array());

$article = new midgard_article(1);
$article->connect("action-update", array(__NAMESPACE__."\callback_class", "my_callback"), array("test1", "test2"));
echo "Found article #1 with name \"" . $article->name . "\" (usually your front page).\n";
echo "Now, touching the article with update().\n\n";
echo "Following two lines should come from the callback handler of action_updated.\n";
echo "(If nothing else than \"update() OK\" prints, you have a bug. :))\n\n";
if ($article->update()) {
    echo "update() OK\n\n";
}
echo "If you didn't get \"update() OK\" please edit this script so that it works in your environment.\n";

?>
