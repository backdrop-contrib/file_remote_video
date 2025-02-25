<?php

/**
 * Implements hook_bee_command().
 */
function file_remote_video_bee_command() {
  $items = [];
  $items['remote-video-refresh-all-metadata'] = [
    'description' => bt('Refresh the metadata for all videos'),
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'callback' => 'file_remote_video_bee_refresh_all_metadata',
  ];
  $items['remote-video-file-refresh-metadata'] = [
    'description' => bt('Refresh the metadata for all videos'),
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'callback' => 'file_remote_video_bee_refresh_file_metadata',
    'arguments' => [
      'fid' => bt('The fid of the file to refresh'),
    ],
  ];
  return $items;
}

function file_remote_video_bee_refresh_all_metadata() {
  $stream_wrappers = file_remote_video_stream_wrappers();
  $types = array_keys($stream_wrappers);
  $mimes = [];
  foreach ($types as $type) {
    $mimes[] = "video/{$type}";
  }
  $sql = <<<EOF
    SELECT
      fid
    FROM
      {file_managed}
    WHERE
     type = 'video'
    AND
     filemime
    IN
     (:mimes)
EOF;
  $query = db_query($sql, [':mimes' => $mimes]);
  while ($fid = $query->fetchField(0)) {
    $file = file_load($fid);
    file_remote_video_fetch_file_metadata($file);
    $file->save();
    sleep(1);
  }
}

function file_remote_video_bee_refresh_file_metadata($arguments, $options) {
  $file = file_load($arguments['fid']);
  if (!file_remote_video_file_is_remote_video($file)) {
    throw new Exception('File is not a remote video');
  }
  file_remote_video_fetch_file_metadata($file);
  $file->save();
}
