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
    $output .= '<div class="file-remote-video-widget-filename">';
    $output .= l($file->filename, "file/{$file->fid}");
    $output .= '</div>';
  }
  $output .= backdrop_render_children($element);
  $output .= '</div>';

  return $output;
}
