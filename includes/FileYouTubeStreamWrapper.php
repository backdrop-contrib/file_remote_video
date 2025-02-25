<?php

class FileYouTubeStreamWrapper extends MediaReadOnlyStreamWrapper {
  protected $base_url = 'https://www.youtube.com/watch';
  public $oembed;

  static function getMimeType($uri, $mapping = NULL) {
    return 'video/youtube';
  }

  function getOEmbed() {
    if (!isset($this->oembed)) {
      $parts = $this->get_parameters();
      $uri = file_stream_wrapper_uri_normalize('youtube://v/' . check_plain($parts['v']));
      $external_url = file_create_url($uri);
      $oembed_url = url('https://www.youtube.com/oembed', array('query' => array('url' => $external_url, 'format' => 'json')));
      $response = drupal_http_request($oembed_url);
      if (!isset($response->error)) {
        $this->oembed = drupal_json_decode($response->data);
      }
      else {
        throw new Exception("Error Processing Request. (Error: {$response->code}, {$response->error})");
        return;
      }
    }
    return $this->oembed;
  }

  function getOriginalThumbnailPath() {
    $parts = $this->get_parameters();
    $thumbnail_url = 'https://img.youtube.com/vi/' . check_plain($parts['v']) . "/maxresdefault.jpg";
    $response = drupal_http_request($thumbnail_url);
    if ($response->code == 404) {
      $thumbnail_url = 'https://img.youtube.com/vi/' . check_plain($parts['v']) . "/hqdefault.jpg";
      $response = drupal_http_request($thumbnail_url);
    }
    if (!isset($response->error)) {
      return $thumbnail_url;
    }
    elseif ($response->code == -110) {
      throw new Exception("Connection timed out.");
    }
    elseif ($response->code == 401) {
      throw new Exception("Embedding has been disabled for this video.");
    }
    elseif ($response->code == 404) {
      return "https://s.ytimg.com/yts/img/image-hh-404-vflvCykRp.png";
    }
    elseif ($response->code != 200) {
      throw new Exception("The YouTube video ID is invalid or the video was deleted.");
    }
    else {
      $data = $this->getOEmbed();
      return $data['thumbnail_url'];
    }
  }

  function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    $id = array_pop($parts);
    $local_path = file_default_scheme() . '://media-youtube/' . check_plain($id) . '.jpg';
    if (!file_exists($local_path)) {
      // getOriginalThumbnailPath throws an exception if there are any errors
      // when retrieving the original thumbnail from YouTube.
      try {
        $dirname = drupal_dirname($local_path);
        file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
        $response = drupal_http_request($this->getOriginalThumbnailPath());

        if (!isset($response->error)) {
          file_unmanaged_save_data($response->data, $local_path, TRUE);
        }
        else {
          system_retrieve_file($this->getOriginalThumbnailPath(), $local_path, FALSE, FILE_EXISTS_REPLACE);
        }
      }
      catch (Exception $e) {
        // In the event of an endpoint error, use the mime type icon provided
        // by the Media module.
        $file = file_uri_to_object($this->uri);
        $icon_dir = variable_get('media_icon_base_directory', 'public://media-icons') . '/' . variable_get('media_icon_set', 'default');
        $local_path = file_icon_path($file, $icon_dir);
      }
    }

    return $local_path;
  }

  /**
   * Updates $base_url depending on whether the embed is a video or playlist.
   */
  function setBaseUrl($parameters) {
    if (isset($parameters['l'])) {
      if (!isset($parameters['v'])) {
        $this->base_url = 'https://youtube.com/playlist';
      }
      $parameters['list'] = $parameters['l'];
      unset($parameters['l']);
    }
    return $parameters;
  }

  /**
   * Returns a url in the format "https://www.youtube.com/watch?v=qsPQN4MiTeE".
   *
   * Overrides interpolateUrl() defined in MediaReadOnlyStreamWrapper.
   */
  function interpolateUrl() {
    if ($parameters = $this->get_parameters()) {
      $parameters = $this->setBaseUrl($parameters);
      return $this->base_url . '?' . http_build_query($parameters);
    }
  }

}
