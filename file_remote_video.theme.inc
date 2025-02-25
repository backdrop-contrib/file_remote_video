<?php

/**
 * Returns HTML for an individual file upload widget.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the widget.
 *
 * @ingroup themeable
 */
function theme_file_remote_video_widget($variables) {
  $element = $variables['element'];
  $output = '';

  $output .= '<div class="file-remote-video-widget clearfix">';
  if ($element['#file']) {
    $file = $element['#file'];
    $thumbnail = file_remote_video_file_get_video_thumbnail($file);
    if ($thumbnail) {
      $output .= '<div class="file-remote-video-widget-thumbnail">';
      $output .= $thumbnail;
      $output .= '</div>';
    }
    $output .= '<div class="file-remote-video-widget-filename">';
    $output .= l($file->filename, "file/{$file->fid}");
    $output .= '</div>';
  }
  $output .= backdrop_render_children($element);
  $output .= '</div>';

  return $output;
}

function theme_file_remote_video_form_thumbnail($variables) {
  $file = $variables['file'];

  $html = '<div class="file-remote-video-manage-file-thumbnail">';
  $html .= '<div><strong>' . t('Thumbnail') . '</strong></div>';

  if (isset($file->metadata) && isset($file->metadata['thumbnail_url'])) {
    $image_vars = [
      'path' => $file->metadata['thumbnail_url'],
      'alt' => $file->filename,
    ];
    if (isset($file->metadata['thumbnail_width'])) {
      $image_vars['width'] = $file->metadata['thumbnail_width'];
    }
    if (isset($file->metadata['thumbnail_height'])) {
      $image_vars['height'] = $file->metadata['thumbnail_height'];
    }
    $html .= theme('image', $image_vars);
  }
  else {
    $html .= '<p><strong>' . t('No thumbnail found') . '</p>';
  }

  $html .= '</div>';

  return $html;
}
