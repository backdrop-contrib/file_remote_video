<?php

/**
 *  @file
 *  Extends the MediaReadOnlyStreamWrapper class to handle Vimeo videos.
 */

/**
 *  Create an instance like this:
 *  $vimeo = new MediaVimeoStreamWrapper('vimeo://v/[video-code]');
 */
class FileVimeoStreamWrapper extends MediaReadOnlyStreamWrapper {
  protected $base_url = 'https://vimeo.com';

  static function getMimeType($uri, $mapping = NULL) {
    return 'video/vimeo';
  }

  function interpolateUrl() {
    if ($parameters = $this->get_parameters()) {
      return $this->base_url . '/' . $parameters['v'];
    }
  }

  public function getOEmbed() {
    if (!isset($this->oembed)) {
      $parts = $this->get_parameters();
      $uri = file_stream_wrapper_uri_normalize('vimeo://v/' . check_plain($parts['v']));
      $external_url = file_create_url($uri);
      $oembed_url = url('http://vimeo.com/api/oembed.json', array('query' => array('url' => $external_url)));
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
    $data = $this->getOEmbed();
    return $data['thumbnail_url'];
  }

  function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    // There's no need to hide thumbnails, always use the public system rather
    // than file_default_scheme().
    $local_path = 'public://media-vimeo/' . check_plain($parts['v']) . '.jpg';

    if (!file_exists($local_path)) {
      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      $response = drupal_http_request($this->getOriginalThumbnailPath());

      if (!isset($response->error)) {
        file_unmanaged_save_data($response->data, $local_path, TRUE);
      }
      else {
        @copy($this->getOriginalThumbnailPath(), $local_path);
      }
    }

    return $local_path;
  }
}
